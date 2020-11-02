// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2015-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/gui/widget_manager.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/ctrl_scroll.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/group_submenu_base.h"
#include "nel/gui/lua_ihm.h"

using namespace NLMISC;
using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

NLMISC_REGISTER_OBJECT(CViewBase, CCtrlScroll, std::string, "scroll");

namespace NLGUI
{

	// ------------------------------------------------------------------------------------------------
	CCtrlScroll::CCtrlScroll(const TCtorParam &param)
	:	CCtrlScrollBase(param)
	{
		_Vertical = true;
		_Aligned = 1;
		_TrackPos = 0;
		_TrackDispPos = 0;
		_TrackSize = _TrackSizeMin = 8;
		_Min = 0;
		_Max = 100;
		_Value = 0;
		_InitialValue = 0;
		_MouseDown = false;
		_CallingAH = false;
		_Cancelable = false;
		_Keyboard = false;
		_Target = NULL;
		_Inverted = false;
		_IsDBLink = false;
		_LastTargetHReal = 0;
		_LastTargetMaxHReal = 0;
		_LastTargetWReal = 0;
		_LastTargetMaxWReal = 0;
		_LastTargetOfsX = 0;
		_LastTargetOfsY = 0;
		_ObserverOn = true;
		_TargetStepX = 1;
		_TargetStepY = 1;
		_StepValue = 0;
		_TileM = false;
		_Frozen = false;
		_Scale = false;
	}

	// ------------------------------------------------------------------------------------------------
	void CCtrlScroll::runAH(const std::string &name, const std::string &params)
	{
		if (name.empty()) return;
		if (_CallingAH) return; // avoid infinite loop
		_CallingAH = true;
		CAHManager::getInstance()->runActionHandler(name, this, params);
		_CallingAH = false;
	}

	// ------------------------------------------------------------------------------------------------
	CCtrlScroll::~CCtrlScroll()
	{
		if (_IsDBLink)
		{
			ICDBNode::CTextId textId;
			_DBLink.getNodePtr()->removeObserver(this, textId);
		}
	}

	std::string CCtrlScroll::getProperty( const std::string &name ) const
	{

		if( name == "tx_bottomleft" )
		{
			return getTextureBottomOrLeft();
		}
		else
		if( name == "tx_middle" )
		{
			return getTextureMiddle();
		}
		else
		if( name == "tx_topright" )
		{
			return getTextureTopOrRight();
		}
		else
		if( name == "scale" )
		{
			return toString( _Scale );
		}
		else
		if( name == "vertical" )
		{
			return toString( _Vertical );
		}
		else
		if( name == "align" )
		{
			switch( _Aligned )
			{
			case 0:
				return "T";
				break;
			case 1:
				return "B";
				break;
			case 2:
				return "L";
				break;
			case 3:
				return "R";
				break;
			}

			return "";
		}
		else
		if( name == "min" )
		{
			return toString( _Min );
		}
		else
		if( name == "max" )
		{
			return toString( _Max );
		}
		else
		if( name == "value" )
		{
			if( _IsDBLink )
				return _DBLink.getNodePtr()->getFullName();
			else
				return toString( _Value );
		}
		else
		if( name == "tracksize" )
		{
			return toString( _TrackSize );
		}
		else
		if( name == "onscroll" )
		{
			return _AHOnScroll;
		}
		else
		if( name == "params" )
		{
			return _AHOnScrollParams;
		}
		else
		if( name == "onscrollend" )
		{
			return _AHOnScrollEnd;
		}
		else
		if( name == "end_params" )
		{
			return _AHOnScrollEndParams;
		}
		else
		if( name == "onscrollcancel" )
		{
			return _AHOnScrollCancel;
		}
		else
		if( name == "cancel_params" )
		{
			return _AHOnScrollCancelParams;
		}
		else
		if( name == "target" )
		{
			if( _Target != NULL )
				return _Target->getId();
			else
				return "";
		}
		else
		if( name == "target_stepx" )
		{
			return toString( _TargetStepX );
		}
		else
		if( name == "target_stepy" )
		{
			return toString( _TargetStepY );
		}
		else
		if( name == "step_value" )
		{
			return toString( _StepValue );
		}
		else
		if( name == "cancelable" )
		{
			return toString( _Cancelable );
		}
		else
		if( name == "keyboard" )
		{
			return toString( _Keyboard );
		}
		else
		if( name == "frozen" )
		{
			return toString( _Frozen );
		}
		else
			return CCtrlBase::getProperty( name );
	}


	void CCtrlScroll::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "tx_bottomleft" )
		{
			setTextureBottomOrLeft( value );
			return;
		}
		else
		if( name == "tx_middle" )
		{
			setTextureMiddle( value );
			return;
		}
		else
		if( name == "tx_topright" )
		{
			setTextureTopOrRight( value );
			return;
		}
		else
		if( name =="scale" )
		{
			bool b;
			if (fromString( value, b ) )
				_Scale = b;
			return;
		}
		else
		if( name == "vertical" )
		{
			bool b;
			if( fromString( value, b ) )
				_Vertical = b;
			return;
		}
		else
		if( name == "align" )
		{
			if( value == "T" )
				_Aligned = 0;
			else
			if( value == "B" )
				_Aligned = 1;
			else
			if( value == "L" )
				_Aligned = 2;
			else
			if( value == "R" )
				_Aligned = 3;
			
			return;
		}
		else
		if( name == "min" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_Min = i;
			return;
		}
		else
		if( name == "max" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_Max = i;
			return;
		}
		else
		if( name == "value" )
		{
			sint32 i;
			if( fromString( value, i ) )
			{
				_IsDBLink = false;
				_Value = i;
			}
			else
			{
				_IsDBLink = true;
				_DBLink.link( value.c_str() );
				ICDBNode::CTextId dummy;
				_DBLink.getNodePtr()->addObserver( this, dummy );
			}
			return;
		}
		else
		if( name == "tracksize" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_TrackSize = i;
			return;
		}
		else
		if( name == "onscroll" )
		{
			_AHOnScroll = value;
			return;
		}
		else
		if( name == "params" )
		{
			_AHOnScrollParams = value;
			return;
		}
		else
		if( name == "onscrollend" )
		{
			_AHOnScrollEnd = value;
			return;
		}
		else
		if( name == "end_params" )
		{
			_AHOnScrollEndParams = value;
			return;
		}
		else
		if( name == "onscrollcancel" )
		{
			_AHOnScrollCancel = value;
			return;
		}
		else
		if( name == "cancel_params" )
		{
			_AHOnScrollCancelParams = value;
			return;
		}
		else
		if( name == "target" )
		{
			_Target = dynamic_cast< CInterfaceGroup* >(
				CWidgetManager::getInstance()->getElementFromId( value )
				);
			return;
		}
		else
		if( name == "target_stepx" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_TargetStepX = i;
			return;
		}
		else
		if( name == "target_stepy" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_TargetStepY = i;
			return;
		}
		else
		if( name == "step_value" )
		{
			uint32 i;
			if( fromString( value, i ) )
				_StepValue = i;
			return;
		}
		else
		if( name == "cancelable" )
		{
			bool b;
			if( fromString( value, b ) )
				_Cancelable = b;
			return;
		}
		else
		if( name == "keyboard" )
		{
			bool b;
			if( fromString( value, b ) )
				_Keyboard = b;
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
			CCtrlBase::setProperty( name, value );
	}

	xmlNodePtr CCtrlScroll::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CCtrlBase::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "scroll" );
		xmlSetProp( node, BAD_CAST "tx_bottomleft", BAD_CAST getTextureBottomOrLeft().c_str() );
		xmlSetProp( node, BAD_CAST "tx_middle", BAD_CAST getTextureMiddle().c_str() );
		xmlSetProp( node, BAD_CAST "tx_topright", BAD_CAST getTextureTopOrRight().c_str() );
		xmlSetProp( node, BAD_CAST "scale", BAD_CAST toString( _Scale ).c_str() );
		xmlSetProp( node, BAD_CAST "vertical", BAD_CAST toString( _Vertical ).c_str() );
		
		std::string align;

		switch( _Aligned )
		{
		case 0:
			align = "T";
			break;

		case 1:
			align = "B";
			break;

		case 2:
			align = "L";
			break;

		case 3:
			align = "R";
			break;
		}
		xmlSetProp( node, BAD_CAST "align", BAD_CAST align.c_str() );

		xmlSetProp( node, BAD_CAST "min", BAD_CAST toString( _Min ).c_str() );
		xmlSetProp( node, BAD_CAST "max", BAD_CAST toString( _Max ).c_str() );

		if( _IsDBLink )
			xmlSetProp( node, BAD_CAST "value", BAD_CAST _DBLink.getNodePtr()->getFullName().c_str() );

		xmlSetProp( node, BAD_CAST "tracksize", BAD_CAST toString( _TrackSize ).c_str() );
		xmlSetProp( node, BAD_CAST "onscroll", BAD_CAST _AHOnScroll.c_str() );
		xmlSetProp( node, BAD_CAST "params", BAD_CAST _AHOnScrollParams.c_str() );
		xmlSetProp( node, BAD_CAST "onscrollend", BAD_CAST _AHOnScrollEnd.c_str() );
		xmlSetProp( node, BAD_CAST "end_params", BAD_CAST _AHOnScrollEndParams.c_str() );
		xmlSetProp( node, BAD_CAST "onscrollcancel", BAD_CAST _AHOnScrollCancel.c_str() );
		xmlSetProp( node, BAD_CAST "cancel_params", BAD_CAST _AHOnScrollCancelParams.c_str() );

		if( _Target != NULL )
			xmlSetProp( node, BAD_CAST "target", BAD_CAST _Target->getId().c_str() );
		else
			xmlSetProp( node, BAD_CAST "target", BAD_CAST "" );

		xmlSetProp( node, BAD_CAST "target_stepx", BAD_CAST toString( _TargetStepX ).c_str() );
		xmlSetProp( node, BAD_CAST "target_stepy", BAD_CAST toString( _TargetStepY ).c_str() );
		xmlSetProp( node, BAD_CAST "step_value", BAD_CAST toString( _StepValue ).c_str() );
		xmlSetProp( node, BAD_CAST "cancelable", BAD_CAST toString( _Cancelable ).c_str() );
		xmlSetProp( node, BAD_CAST "keyboard", BAD_CAST toString( _Keyboard ).c_str() );
		xmlSetProp( node, BAD_CAST "frozen", BAD_CAST toString( _Frozen ).c_str() );

		return node;
	}

	// ------------------------------------------------------------------------------------------------
	bool CCtrlScroll::parse(xmlNodePtr node, CInterfaceGroup * parentGroup)
	{
		if (!CCtrlBase::parse(node, parentGroup))
			return false;

		CXMLAutoPtr prop;
		// Read textures
		prop = (char*) xmlGetProp( node, (xmlChar*)"tx_bottomleft" );
		if(prop) setTextureBottomOrLeft(string((const char*)prop));
		else setTextureBottomOrLeft ("w_scroll_l0_b.tga");

		prop = (char*) xmlGetProp( node, (xmlChar*)"tx_middle" );
		if(prop)	setTextureMiddle(string((const char*)prop));
		else setTextureMiddle ("w_scroll_l0_m.tga");

		prop = (char*) xmlGetProp( node, (xmlChar*)"tx_topright" );
		if(prop) setTextureTopOrRight(string((const char*)prop));
		else setTextureTopOrRight ("w_scroll_l0_t.tga");

		// Override texture size (w for vertical, h for horizontal)
		prop = (char*) xmlGetProp( node, (xmlChar*)"scale" );
		if (prop) _Scale = convertBool((const char*)prop);

		// Read properties
		prop = (char*) xmlGetProp( node, (xmlChar*)"vertical" );
		if (prop) _Vertical = convertBool((const char*)prop);

		prop = (char*) xmlGetProp (node, (xmlChar*)"align");
		_Aligned = 1;
		if (prop)
		{
			if (stricmp(prop, "T") == 0) _Aligned = 0;
			else if (stricmp(prop, "B") == 0) _Aligned = 1;
			else if (stricmp(prop, "L") == 0) _Aligned = 2;
			else if (stricmp(prop, "R") == 0) _Aligned = 3;
		}

		prop = (char*) xmlGetProp( node, (xmlChar*)"min" );
		if (prop) fromString((const char*)prop, _Min);

		prop = (char*) xmlGetProp( node, (xmlChar*)"max" );
		if (prop) fromString((const char*)prop, _Max);

		prop = (char*) xmlGetProp( node, (xmlChar*)"value" );
		if (prop)
		{
			if ( isdigit(*prop) || *prop=='-')
			{
				_IsDBLink = false;
				fromString((const char*)prop, _Value);
			}
			else
			{
				_IsDBLink = true;
				_DBLink.link(prop);
				ICDBNode::CTextId textId;
				_DBLink.getNodePtr()->addObserver(this, textId);

			}
		}

		prop = (char*) xmlGetProp( node, (xmlChar*)"tracksize" );
		if (prop) fromString((const char*)prop, _TrackSize);

		// Read Action handlers
		prop = (char*) xmlGetProp( node, (xmlChar*)"onscroll" );
		if (prop)	_AHOnScroll = NLMISC::toLowerAscii(prop.str());
		prop = (char*) xmlGetProp( node, (xmlChar*)"params" );
		if (prop)	_AHOnScrollParams = string((const char*)prop);
		//
		prop = (char*) xmlGetProp( node, (xmlChar*)"onscrollend" );
		if (prop)	_AHOnScrollEnd = NLMISC::toLowerAscii(prop.str());
		prop = (char*) xmlGetProp( node, (xmlChar*)"end_params" );
		if (prop)	_AHOnScrollEndParams = string((const char*)prop);
		//
		prop = (char*) xmlGetProp( node, (xmlChar*)"onscrollcancel" );
		if (prop)	_AHOnScrollCancel = NLMISC::toLowerAscii(prop.str());
		prop = (char*) xmlGetProp( node, (xmlChar*)"cancel_params" );
		if (prop)	_AHOnScrollCancelParams = string((const char*)prop);


		// auto-target
		prop = (char*) xmlGetProp( node, (xmlChar*)"target" );
		if (prop)
		{
			CInterfaceGroup	*group = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(prop.str()));
			if(group == NULL)
				group = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(this->getId(), prop.str()));

			if(group != NULL)
				setTarget (group);
		}


		// auto-step
		prop = (char*) xmlGetProp( node, (xmlChar*)"target_stepx" );
		if(prop)	fromString((const char*)prop, _TargetStepX);
		prop = (char*) xmlGetProp( node, (xmlChar*)"target_stepy" );
		if(prop)	fromString((const char*)prop, _TargetStepY);
		_TargetStepX= max((sint32)1, _TargetStepX);
		_TargetStepY= max((sint32)1, _TargetStepY);

		// Scroll Step
		prop = (char*) xmlGetProp( node, (xmlChar*)"step_value" );
		if(prop)	fromString((const char*)prop, _StepValue);

		prop = (char*) xmlGetProp( node, (xmlChar*)"cancelable" );
		if (prop) _Cancelable = convertBool(prop);

		prop = (char*) xmlGetProp( node, (xmlChar*)"keyboard" );
		if (prop) _Keyboard = convertBool(prop);

		prop= (char*) xmlGetProp (node, (xmlChar*)"frozen");
		_Frozen = false;
		if (prop)
			_Frozen = convertBool(prop);


		return true;
	}

	// ------------------------------------------------------------------------------------------------

	int CCtrlScroll::luaSetTarget(CLuaState &ls)
	{
		const char *funcName = "setTarget";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
		std::string targetId = ls.toString(1);

		CInterfaceGroup	*group = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(targetId));
		if(group != NULL)
		{
			setTarget (group);
		}

		return 0;
	}

	void CCtrlScroll::updateCoords()
	{
		if (_Target)
		{
			// update only if visible
			if (_Target->getActive())
			{
				CViewRenderer &rVR = *CViewRenderer::getInstance();
				sint32 w, h;
				rVR.getTextureSizeFromId (_TxIdB, w, h);

				if (_Vertical)
				{
					if (!_Scale) _W = w;
					_H = _Target->getMaxHReal();
				}
				else
				{
					_W = _Target->getMaxWReal();
					if (!_Scale) _H = h;
				}

				CCtrlBase::updateCoords ();
				if (_Vertical)
				{
					if(_Target->getHReal()!=_LastTargetHReal || _Target->getMaxHReal()!=_LastTargetMaxHReal ||
					   _Target->getOfsY() != _LastTargetOfsY
					  )
					{
						_LastTargetHReal= _Target->getHReal();
						_LastTargetMaxHReal= _Target->getMaxHReal();
						_LastTargetOfsY = _Target->getOfsY();

						// Activate only if needed
						setActive(_Target->getHReal() > _Target->getMaxHReal());
						CCtrlBase::updateCoords();

						// Calculate size of the track button
						if ((_Target->getHReal() <= _Target->getMaxHReal()) || (_Target->getHReal() == 0))
						{
							_TrackSize = _Target->getMaxHReal();
						}
						else
						{
							float factor = (float)_Target->getMaxHReal() / (float)_Target->getHReal();
							factor = _TrackSizeMin + factor * (this->getHReal() - _TrackSizeMin);
							_TrackSize = (sint32)factor;
						}
						// Calculate pos of the track button
						if (_Target->getHReal() <= _Target->getMaxHReal())
						{
							if (_Aligned == 1) // BOTTOM
								_TrackPos = 0;
							else // TOP
								_TrackPos = getHReal()-_TrackSize;
						}
						else
						{
							if (_Aligned == 1) // BOTTOM
							{
								float factor = ((float)_Target->getHReal() - (float)_Target->getMaxHReal());
								factor = -(float)_Target->getOfsY() / factor;
								if (factor < 0.0f) factor = 0.0f;
								if (factor > 1.0f) factor = 1.0f;
								_TrackPos = factor * (getHReal()-_TrackSize);
							}
							else // TOP
							{
								float factor = ((float)_Target->getHReal() - (float)_Target->getMaxHReal());
								factor = (float)_Target->getOfsY() / factor;
								if (factor < 0.0f) factor = 0.0f;
								if (factor > 1.0f) factor = 1.0f;
								sint32 hreal = getHReal();
								_TrackPos = (1.0f-factor) * (hreal - _TrackSize);
							}
						}

						// invalidate coords.
						computeTargetOfsFromPos();
					}
				}
				else // Horizontal Tracker
				{
					if(_Target->getWReal()!=_LastTargetWReal || _Target->getMaxWReal()!=_LastTargetMaxWReal ||
					   _Target->getOfsX() != _LastTargetOfsX)
					{
						_LastTargetWReal= _Target->getWReal();
						_LastTargetMaxWReal= _Target->getMaxWReal();
						_LastTargetOfsX = _Target->getOfsX();

						// Activate only if needed
						setActive(_Target->getWReal() > _Target->getMaxWReal());
						CCtrlBase::updateCoords();

						// Calculate size of the track button
						if ((_Target->getWReal() <= _Target->getMaxWReal()) || (_Target->getWReal() == 0))
						{
							_TrackSize = _Target->getMaxWReal();
						}
						else
						{
							float factor = (float)_Target->getMaxWReal() / (float)_Target->getWReal();
							factor = _TrackSizeMin + factor * (this->getWReal() - _TrackSizeMin);
							_TrackSize = (sint32)factor;
						}
						// Calculate pos of the track button
						if (_Target->getWReal() <= _Target->getMaxWReal())
						{
							if (_Aligned == 2) // LEFT
								_TrackPos = 0;
							else // RIGHT
								_TrackPos = getWReal()-_TrackSize;
						}
						else
						{
							if (_Aligned == 2) // LEFT
							{
								float factor = ((float)_Target->getWReal() - (float)_Target->getMaxWReal());
								factor = -(float)_Target->getOfsX() / factor;
								if (factor < 0.0f) factor = 0.0f;
								if (factor > 1.0f) factor = 1.0f;
								_TrackPos = factor * (getWReal()-_TrackSize);
							}
							else // RIGHT
							{
								float factor = ((float)_Target->getWReal() - (float)_Target->getMaxWReal());
								factor = (float)_Target->getOfsX() / factor;
								if (factor < 0.0f) factor = 0.0f;
								if (factor > 1.0f) factor = 1.0f;
								sint32 hreal = getWReal();
								_TrackPos = (1.0f-factor) * (hreal - _TrackSize);
							}
						}

						// invalidate coords.
						computeTargetOfsFromPos();
					}
				}
			}
			// reset cache
			else
			{
				_LastTargetHReal= 0;
				_LastTargetMaxHReal= 0;
				_LastTargetWReal= 0;
				_LastTargetMaxWReal= 0;
				_LastTargetOfsX = 0;
				_LastTargetOfsY = 0;
				setActive(false);
			}
		}
		else
		{
			CCtrlBase::updateCoords ();
			if (_IsDBLink)
				_Value = _DBLink.getSInt32();
			if (_Vertical)
			{
				float factor;
				if (_Aligned == 1) // BOTTOM
					factor = ((float)_Value-_Min) / (_Max-_Min);
				else // TOP
					factor = 1.0f - ((float)_Value-_Min) / (_Max-_Min);
				factor *= (this->getHReal() - _TrackSize);

				_TrackDispPos = (sint32)factor;
			}
			else
			{
				float factor;
				if (_Aligned == 2) // LEFT
					factor = ((float)_Value-_Min) / (_Max-_Min);
				else // RIGHT
					factor = 1.0f - ((float)_Value-_Min) / (_Max-_Min);
				factor *= (this->getWReal() - _TrackSize);

				_TrackDispPos = (sint32)factor;
			}
		}
		CCtrlBase::updateCoords ();
	}

	// ------------------------------------------------------------------------------------------------
	void CCtrlScroll::draw()
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		CRGBA col = CWidgetManager::getInstance()->getGlobalColorForContent();

		if (_Target)
		{
			if (_Vertical)
			{
				rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal+_TrackPos, _WReal, 4, 0, false, _TxIdB, col );
				if (_TileM == 0)
					rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal+_TrackPos+4, _WReal, _TrackSize-8, 0, false, _TxIdM, col );
				else
					rVR.drawRotFlipBitmapTiled (_RenderLayer, _XReal, _YReal+_TrackPos+4, _WReal, _TrackSize-8, 0, false, _TxIdM, _TileM-1, col );
				rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal+_TrackPos+_TrackSize-4, _WReal, 4, 0, false, _TxIdT, col );
			}
			else
			{
				rVR.drawRotFlipBitmap (_RenderLayer, _XReal+_TrackPos, _YReal, 4, _HReal, 0, false, _TxIdB, col );
				if (_TileM == 0)
					rVR.drawRotFlipBitmap (_RenderLayer, _XReal+_TrackPos+4, _YReal, _TrackSize-8, _HReal, 0, false, _TxIdM, col );
				else
					rVR.drawRotFlipBitmapTiled (_RenderLayer, _XReal+_TrackPos+4, _YReal, _TrackSize-8, _HReal, 0, false, _TxIdM, _TileM-1, col );
				rVR.drawRotFlipBitmap (_RenderLayer, _XReal+_TrackPos+_TrackSize-4, _YReal, 4, _HReal, 0, false, _TxIdT, col );
			}
		}
		else
		{
			if (_Vertical)
			{
				rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal+_TrackDispPos, _WReal, 4, 0, false, _TxIdB, col );
				if (_TileM == 0)
					rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal+_TrackDispPos+4, _WReal, _TrackSize-8, 0, false, _TxIdM, col );
				else
					rVR.drawRotFlipBitmapTiled (_RenderLayer, _XReal, _YReal+_TrackDispPos+4, _WReal, _TrackSize-8, 0, false, _TxIdM, _TileM-1, col );
				rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal+_TrackDispPos+_TrackSize-4, _WReal, 4, 0, false, _TxIdT, col );
			}
			else
			{
				rVR.drawRotFlipBitmap (_RenderLayer, _XReal+_TrackDispPos, _YReal, 4, _HReal, 0, false, _TxIdB, col );
				if (_TileM == 0)
					rVR.drawRotFlipBitmap (_RenderLayer, _XReal+_TrackDispPos+4, _YReal, _TrackSize-8, _HReal, 0, false, _TxIdM, col );
				else
					rVR.drawRotFlipBitmapTiled (_RenderLayer, _XReal+_TrackDispPos+4, _YReal, _TrackSize-8, _HReal, 0, false, _TxIdM, _TileM-1, col );
				rVR.drawRotFlipBitmap (_RenderLayer, _XReal+_TrackDispPos+_TrackSize-4, _YReal, 4, _HReal, 0, false, _TxIdT, col );
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	bool CCtrlScroll::handleEvent (const NLGUI::CEventDescriptor &event)
	{
		if (CCtrlBase::handleEvent(event)) return true;
		if (!_Active || _Frozen)
			return false;
		if (event.getType() == NLGUI::CEventDescriptor::mouse)
		{
			const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;
			if ((CWidgetManager::getInstance()->getCapturePointerLeft() != this) &&
				(!((eventDesc.getX() >= _XReal) &&
				(eventDesc.getX() < (_XReal + _WReal))&&
				(eventDesc.getY() > _YReal) &&
				(eventDesc.getY() <= (_YReal+ _HReal)))))
				return false;

			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftdown)
			{
				_MouseDown = true;
				_InitialValue = getValue();
				if (!_Target)
					_TrackPos = _TrackDispPos;
				_MouseDownOffsetX = eventDesc.getX() - (getXReal() + (_Vertical ? 0 : _TrackPos));
				_MouseDownOffsetY = eventDesc.getY() - (getYReal() + (_Vertical ? _TrackPos : 0));

				// if target is a menu, hidde its sub menus
				if(_Target && _Target->getParent())
				{
					CGroupSubMenuBase *menu = dynamic_cast<CGroupSubMenuBase*>(_Target->getParent());
					if(menu)
						menu->hideSubMenus();
				}
				return true;
			}
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup)
			{
				_MouseDown = false;
				runAH(_AHOnScrollEnd, _AHOnScrollEndParams.empty() ? _AHOnScrollParams : _AHOnScrollEndParams); // backward compatibility
				return true;
			}
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightdown && _MouseDown && _Cancelable)
			{
				_MouseDown = false;
				setValue(_InitialValue);
				runAH(_AHOnScrollCancel, _AHOnScrollCancelParams); // backward compatibility
				return true;
			}
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousemove)
			{
				if (_MouseDown)
				{
					sint32 dx = eventDesc.getX() - (getXReal() + (_Vertical ? 0 : _TrackPos) + _MouseDownOffsetX);
					sint32 dy = eventDesc.getY() - (getYReal() + (_Vertical ? _TrackPos : 0 ) + _MouseDownOffsetY);
					if (dx != 0) moveTrackX (dx);
					if (dy != 0) moveTrackY (dy);
				}
				return true;
			}
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousewheel && _Vertical)
			{
				moveTargetY (-(eventDesc.getWheel() * 12));
				return true;
			}
		}
		else if (event.getType() == NLGUI::CEventDescriptor::key)
		{
			const NLGUI::CEventDescriptorKey &eventDesc = (const NLGUI::CEventDescriptorKey &)event;

			if (eventDesc.getKeyEventType() == NLGUI::CEventDescriptorKey::keydown)
			{
				if (_Keyboard)
				{
					sint32 i = 0;
					// direction
					if (eventDesc.getKey() == KeyNEXT)  i++;
					else if (eventDesc.getKey() == KeyPRIOR) i--;
					else
						return false;

					if (_Vertical)
						moveTrackY(-(i * _TargetStepY));
					else
						moveTrackX(-(i * _TargetStepX));

					return true;
				}
			}
		}
		return false;
	}

	// ------------------------------------------------------------------------------------------------
	void CCtrlScroll::setTarget (CInterfaceGroup *pIG)
	{
		_Target = pIG;

		if (_Vertical)
		{
			if (_Target->getPosRef()&Hotspot_Tx)
				_Aligned = 0;
			else
				_Aligned = 1;

			if (_Target->getPosRef()&Hotspot_Tx)
				_Inverted = true;
			else
				_Inverted = true;
		}
		else
		{
			if (_Target->getPosRef()&Hotspot_xL)
				_Aligned = 2;
			else
				_Aligned = 3;

			if (_Target->getPosRef()&Hotspot_xL)
				_Inverted = true;
			else
				_Inverted = true;
		}
	}
	// ------------------------------------------------------------------------------------------------
	sint32 CCtrlScroll::moveTrackX (sint32 dx)
	{
		if (_Vertical)
			return 0;
		if ((getWReal()-_TrackSize) <= 0)
			return 0;

		float newtpos;
		float tpos = _TrackPos;
		sint32 tsize = _TrackSize;

		// Limit the scroller to the defined area
		newtpos = tpos + dx;
		if (newtpos < 0) newtpos = 0;
		if (newtpos > (getWReal()-tsize)) newtpos = (getWReal()-tsize);
		dx = newtpos - tpos;

		if (_Target)
		{
			_TrackPos = newtpos;

			computeTargetOfsFromPos();
		}
		else // This is a number scroller
		{
			float factor = (float)(_Max - _Min);

			if (_Aligned == 2) // LEFT
				factor = -factor * newtpos / (float)(getWReal()-tsize) - _Min;
			else // RIGHT
				factor = factor * (1.0f-(newtpos / (float)(getWReal()-tsize))) + _Min;

			_TrackPos = newtpos;

			if (_Aligned == 2) // LEFT
				_Value = (sint32) (_Inverted ? factor : -factor);
			else // RIGHT
				_Value = (sint32) (_Inverted ? factor : -factor);

			// step and clamp value
			normalizeValue(_Value);

			{
				float factor;
				if (_Aligned == 2) // LEFT
					factor = ((float)_Value-_Min) / (_Max-_Min);
				else // RIGHT
					factor = 1.0f - ((float)_Value-_Min) / (_Max-_Min);
				factor *= (this->getWReal() - _TrackSize);

				_TrackDispPos = (sint32)factor;
			}

			if (_IsDBLink)
			{
				_ObserverOn = false;
				_DBLink.setSInt32 (_Value);
				_ObserverOn = true;
			}
		}

		// Launch the scroller event if any
		runAH(_AHOnScroll, _AHOnScrollParams);

		return dx;
	}

	// ------------------------------------------------------------------------------------------------
	sint32 CCtrlScroll::moveTrackY (sint32 dy)
	{
		if (!_Vertical)
			return 0;
		if ((getHReal()-_TrackSize) <= 0)
			return 0;

		float newtpos;
		float tpos = _TrackPos;
		sint32 tsize = _TrackSize;

		// Limit the scroller to the defined area
		newtpos = tpos + dy;
		if (newtpos < 0) newtpos = 0;
		if (newtpos > (getHReal()-tsize)) newtpos = (getHReal()-tsize);
		dy = newtpos - tpos;

		if (_Target)
		{
			_TrackPos = newtpos;

			computeTargetOfsFromPos();
		}
		else // This is a number scroller
		{
			float factor = (float)(_Max - _Min);

			if (_Aligned == 1) // BOTTOM
				factor = -factor * newtpos / (float)(getHReal()-tsize) - _Min;
			else // TOP
				factor = factor * (1.0f-(newtpos / (float)(getHReal()-tsize))) + _Min;

			_TrackPos = newtpos;

			if (_Aligned == 1) // BOTTOM
				_Value = (sint32) (_Inverted ? factor : -factor);
			else // TOP
				_Value = (sint32) (_Inverted ? factor : -factor);

			// step and clamp value
			normalizeValue(_Value);

			{
				float factor;
				if (_Aligned == 1) // BOTTOM
					factor = ((float)_Value-_Min) / (_Max-_Min);
				else // TOP
					factor = 1.0f - ((float)_Value-_Min) / (_Max-_Min);
				factor *= (this->getHReal() - _TrackSize);

				_TrackDispPos = (sint32)factor;
			}

			if (_IsDBLink)
			{
				_ObserverOn = false;
				_DBLink.setSInt32 (_Value);
				_ObserverOn = true;
			}
		}

		// Launch the scroller event if any
		runAH(_AHOnScroll, _AHOnScrollParams);

		return dy;
	}

	// ------------------------------------------------------------------------------------------------
	void CCtrlScroll::setTextureBottomOrLeft (const std::string &txName)
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		_TxIdB = rVR.getTextureIdFromName(txName);
	}

	// ------------------------------------------------------------------------------------------------
	void CCtrlScroll::setTextureMiddle (const std::string &txName)
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		_TxIdM = rVR.getTextureIdFromName(txName);
	}

	// ------------------------------------------------------------------------------------------------
	void CCtrlScroll::setTextureTopOrRight (const std::string &txName)
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		_TxIdT = rVR.getTextureIdFromName(txName);
	}

	std::string CCtrlScroll::getTextureBottomOrLeft() const
	{
		return CViewRenderer::getInstance()->getTextureNameFromId( _TxIdB );
	}

	std::string CCtrlScroll::getTextureMiddle() const
	{
		return CViewRenderer::getInstance()->getTextureNameFromId( _TxIdM );
	}

	std::string CCtrlScroll::getTextureTopOrRight() const
	{
		return CViewRenderer::getInstance()->getTextureNameFromId( _TxIdT );
	}

	// ------------------------------------------------------------------------------------------------
	void CCtrlScroll::setValue(sint32 value)
	{
		normalizeValue(value);

		if (_IsDBLink)
		{
			_ObserverOn = false;
			_DBLink.setSInt32(value);
			_ObserverOn = true;
		}
		else
		{
			_Value = value;
		}
		invalidateCoords();
	}

	// ------------------------------------------------------------------------------------------------
	void CCtrlScroll::setTrackPos(sint32 pos)
	{
		if (_Vertical)
		{
			moveTrackY(pos - _TrackPos);
		}
		else
		{
			moveTrackX(pos - _TrackPos);
		}
		invalidateCoords();
	}

	// ------------------------------------------------------------------------------------------------
	void CCtrlScroll::computeTargetOfsFromPos()
	{
		if(_Vertical)
		{
			float factor = ((float)_Target->getHReal() - (float)_Target->getMaxHReal());

			if (_Aligned == 1) // BOTTOM
				factor = -factor * _TrackPos / favoid0((float)(getHReal()-_TrackSize));
			else // TOP
				factor = factor * (1.0f-(_TrackPos / favoid0((float)(getHReal()-_TrackSize))));

			// Compute Steped target
			sint32	nexOfsY= (sint32) floorf(0.5+(_Inverted ? factor : -factor));
			if(_TargetStepY>1)
				nexOfsY= ((nexOfsY+_TargetStepY/2)/_TargetStepY) * _TargetStepY;
			_Target->setOfsY (nexOfsY);
			_LastTargetOfsY = nexOfsY;

			// invalidate only XReal/YReal, doing only 1 pass
			_Target->invalidateCoords(1);
		}
		else
		{
			float factor = ((float)_Target->getWReal() - (float)_Target->getMaxWReal());

			if (_Aligned == 2) // LEFT
				factor = -factor * _TrackPos / favoid0((float)(getWReal()-_TrackSize));
			else // RIGHT
				factor = factor * (1.0f-(_TrackPos / favoid0((float)(getWReal()-_TrackSize))));

			// Compute Steped target
			sint32	nexOfsX= (sint32) floorf(0.5+(_Inverted ? factor : -factor));
			if(_TargetStepX>1)
				nexOfsX= ((nexOfsX+_TargetStepX/2)/_TargetStepX) * _TargetStepX;
			_Target->setOfsX (nexOfsX);
			_LastTargetOfsX = nexOfsX;

			// invalidate only XReal/YReal, doing only 1 pass
			_Target->invalidateCoords(1);
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CCtrlScroll::update(ICDBNode * /* node */)
	{
		if (!_ObserverOn) return;
		_Value = _DBLink.getSInt32();
		// the value in the db changed
		invalidateCoords(1);
		if (_Target) _Target->invalidateCoords(1);
	}


	// ***************************************************************************
	void	CCtrlScroll::moveTargetX (sint32 dx)
	{
		if(!_Target)
			return;
		sint32	maxWReal= _Target->getMaxWReal();
		sint32	wReal= _Target->getWReal();
		if(wReal <= maxWReal)
			return;

		// compute the new ofsX.
		sint32	ofsX= _Target->getOfsX();
		ofsX+= dx;
		clamp(ofsX, 0, wReal-maxWReal);
		_Target->setOfsX(ofsX);

		// compute new trackPos.
		if (_Aligned == 2) // LEFT
		{
			float factor = (float)(wReal-maxWReal);
			factor = -(float)ofsX / factor;
			clamp(factor, 0.f, 1.f);
			_TrackPos = factor * (getWReal()-_TrackSize);
		}
		else // RIGHT
		{
			float factor = (float)(wReal-maxWReal);
			factor = (float)ofsX / factor;
			clamp(factor, 0.f, 1.f);
			_TrackPos = (1.0f-factor) * (getWReal() - _TrackSize);
		}

		// invalidate only position. 1 pass is sufficient
		invalidateCoords(1);
	}

	// ***************************************************************************
	void	CCtrlScroll::moveTargetY (sint32 dy)
	{
		if(!_Target)
			return;
		sint32	maxHReal= _Target->getMaxHReal();
		sint32	hReal= _Target->getHReal();
		if(hReal <= maxHReal)
			return;

		if (_TargetStepY > 1)
		{
			sint sign = (0 < dy) - (dy < 0);
			dy = sign * max(1, (dy / _TargetStepY)) * _TargetStepY;
		}

		// compute the new ofsY.
		sint32	ofsY= _Target->getOfsY();
		ofsY+= dy;

		// compute new trackPos.
		if (_Aligned == 1) // BOTTOM
		{
			clamp(ofsY, maxHReal - hReal, 0);
			_Target->setOfsY(ofsY);
			float factor = (float)(hReal-maxHReal);
			factor = -(float)ofsY / factor;
			clamp(factor, 0.f, 1.f);
			_TrackPos = factor * (getHReal()-_TrackSize);
		}
		else // TOP
		{
			clamp(ofsY, 0, hReal-maxHReal);
			_Target->setOfsY(ofsY);
			float factor = (float)(hReal-maxHReal);
			factor = (float)ofsY / factor;
			clamp(factor, 0.f, 1.f);
			_TrackPos = (1.0f-factor) * (getHReal() - _TrackSize);
		}

		// invalidate only position. 1 pass is sufficient
		invalidateCoords(1);
	}

	// ***************************************************************************
	void	CCtrlScroll::normalizeValue(sint32 &value)
	{
		// if, 0 no step
		if(_StepValue==0 || _StepValue==1)
			return;
		// if interval is null, force min!
		if(_Max==_Min)
		{
			value= _Min;
			return;
		}

		// get range of possible position
	//	sint32	size= _Max - _Min;

		// step (round)
		sint32	val= (value + (_StepValue/2) -_Min) / _StepValue;
		val= _Min + val * _StepValue;
		clamp(val, _Min, _Max);
		value= val;
	}

	// ***************************************************************************
	void CCtrlScroll::setFrozen (bool state)
	{
		_Frozen = state;
		if (_Frozen)
		{
			_Value = 0;
		}
	}


	// ------------------------------------------------------------------------------------------------
	int CCtrlScroll::luaEnsureVisible(CLuaState &ls)
	{
		const char *funcName = "ensureVisible";
		CLuaIHM::checkArgCount(ls, funcName, 3);
		CLuaIHM::checkArgTypeUIElement(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
		CLuaIHM::checkArgType(ls, funcName, 3, LUA_TSTRING);
		THotSpot	hs[2];
		std::string hsStr[] = { ls.toString(2), ls.toString(3) };
		//
		for (uint hsIndex = 0; hsIndex < 2; ++ hsIndex)
		{
			if (_Vertical)
			{
				if (NLMISC::nlstricmp(hsStr[hsIndex], "T") == 0)
				{
					hs[hsIndex] = Hotspot_Tx;
				}
				else
				if (NLMISC::nlstricmp(hsStr[hsIndex], "M") == 0)
				{
					hs[hsIndex] = Hotspot_Mx;
				}
				else
				if (NLMISC::nlstricmp(hsStr[hsIndex], "B") == 0)
				{
					hs[hsIndex] = Hotspot_Bx;
				}
				else
				{
					hs[hsIndex] = Hotspot_Bx;
					CLuaIHM::fails(ls, "%s : couldn't parse hotspot for vertical scrollbar", funcName);
				}
			}
			else
			{
				if (NLMISC::nlstricmp(hsStr[hsIndex], "L") == 0)
				{
					hs[hsIndex] = Hotspot_xL;
				}
				else
				if (NLMISC::nlstricmp(hsStr[hsIndex], "M") == 0)
				{
					hs[hsIndex] = Hotspot_xM;
				}
				else
				if (NLMISC::nlstricmp(hsStr[hsIndex], "R") == 0)
				{
					hs[hsIndex] = Hotspot_xR;
				}
				else
				{
					hs[hsIndex] = Hotspot_xL;
					CLuaIHM::fails(ls, "%s : couldn't parse hotspot for horizontal scrollbar", funcName);
				}
			}
		}
		ensureVisible(CLuaIHM::getUIOnStack(ls, 1), hs[0], hs[1]);
		return 0;
	}


	// ------------------------------------------------------------------------------------------------
	void CCtrlScroll::ensureVisible(CInterfaceElement *childElement, THotSpot childHotSpot, THotSpot parentHotSpot)
	{
		if (!_Target) return; // not connected to a target yet
		if (!childElement) return;
		// compute corners of interest for enclosed element & enclosing group
		sint32 childX, childY;
		childElement->getCorner(childX, childY, childHotSpot);
		if (_Vertical)
		{
			sint32	maxHReal= _Target->getMaxHReal();
			sint32	hReal=	  _Target->getHReal();
			if(hReal > maxHReal)
			{
				sint enclosingDY;
				switch (parentHotSpot)
				{
					case Hotspot_Bx:
						enclosingDY = maxHReal;
					break;
					case Hotspot_Mx:
						enclosingDY = maxHReal / 2;
					break;
					case Hotspot_Tx:
						enclosingDY = 0;
					break;
					default:
						nlassert(0);
					break;
				}
				if (_Aligned == 0)
				{
					// Top aligned case
					sint32 offsetY = (_Target->getYReal() + _Target->getHReal() - childY) - enclosingDY;
					NLMISC::clamp(offsetY, 0, hReal - maxHReal);
					_Target->setOfsY(offsetY);
					_Target->invalidateCoords();
				}
				else if (_Aligned == 1)
				{
					// Bottom aligned case
					sint32 offsetY = (maxHReal - enclosingDY) - (childY - _Target->getYReal());
					NLMISC::clamp(offsetY, maxHReal - hReal, 0);
					_Target->setOfsY(offsetY);
					_Target->invalidateCoords();
				}
			}
			// else, ... fully visible (not occluded by parent group)
		}
		else
		{
			sint32	maxWReal= _Target->getMaxWReal();
			sint32	wReal=	  _Target->getWReal();
			if(wReal > maxWReal)
			{
				sint enclosingDX;
				switch (parentHotSpot)
				{
					case Hotspot_xL:
						enclosingDX = maxWReal;
					break;
					case Hotspot_xM:
						enclosingDX = maxWReal / 2;
					break;
					case Hotspot_xR:
						enclosingDX = 0;
					break;
					default:
						nlassert(0);
					break;
				}
				if (_Aligned == 3)
				{
					// right aligned case
					sint32 offsetX = (_Target->getXReal() + _Target->getWReal() - childX) - enclosingDX;
					NLMISC::clamp(offsetX, 0, wReal - maxWReal);
					_Target->setOfsX(offsetX);
					_Target->invalidateCoords();
				}
				else if (_Aligned == 2)
				{
					// Left aligned case
					sint32 offsetX = (maxWReal - enclosingDX) - (childX - _Target->getXReal());
					NLMISC::clamp(offsetX, maxWReal - wReal, 0);
					_Target->setOfsX(offsetX);
					_Target->invalidateCoords();
				}
			}
			// else, ... fully visible (not occluded by parent group)
		}
	}
}

