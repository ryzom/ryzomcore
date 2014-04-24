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
#include "nel/gui/ctrl_col_pick.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/interface_group.h"


using namespace NLMISC;
using namespace std;

NLMISC_REGISTER_OBJECT(CViewBase, CCtrlColPick, std::string, "colpick");

namespace NLGUI
{

	// ------------------------------------------------------------------------------------------------
	CCtrlColPick::CCtrlColPick(const TCtorParam &param)
	:CCtrlBase(param)
	{
		_MouseDown = false;
		_Texture = -2;
	}

	// ------------------------------------------------------------------------------------------------
	CCtrlColPick::~CCtrlColPick()
	{
		// Texture has been created ?
		if (_Texture>=0)
		{
			CViewRenderer &rVR = *CViewRenderer::getInstance();
			rVR.deleteTexture (_Texture);
		}
	}

	std::string CCtrlColPick::getProperty( const std::string &name ) const
	{
		if( name == "texture" )
		{
			return CViewRenderer::getInstance()->getTextureNameFromId( _Texture );
		}
		else
		if( name == "onchange" )
		{
			return _AHOnChange;
		}
		else
		if( name == "onchange_params" )
		{
			return _AHOnChangeParams;
		}
		else
		if( name == "dbcolr" )
		{
			if( _ColSelR.getNodePtr() != NULL )
				return _ColSelR.getNodePtr()->getFullName();
			else
				return "";
		}
		else
		if( name == "dbcolg" )
		{
			if( _ColSelG.getNodePtr() != NULL )
				return _ColSelG.getNodePtr()->getFullName();
			else
				return "";
		}
		else
		if( name == "dbcolb" )
		{
			if( _ColSelB.getNodePtr() != NULL )
				return _ColSelB.getNodePtr()->getFullName();
			else
				return "";
		}
		else
		if( name == "dbcola" )
		{
			if( _ColSelA.getNodePtr() != NULL )
				return _ColSelA.getNodePtr()->getFullName();
			else
				return "";
		}
		else
			return CCtrlBase::getProperty( name );
	}

	void CCtrlColPick::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "texture" )
		{
			CViewRenderer::getInstance()->deleteTexture( _Texture );
			_Texture = CViewRenderer::getInstance()->createTexture( value, 0, 0, 256, 64, false, false );
			return;
		}
		else
		if( name == "onchange" )
		{
			_AHOnChange = value;
		}
		else
		if( name == "onchange_params" )
		{
			_AHOnChangeParams = value;
			return;
		}
		else
		if( name == "dbcolr" )
		{
			_ColSelR.link( value.c_str() );
			return;
		}
		else
		if( name == "dbcolg" )
		{
			_ColSelG.link( value.c_str() );
			return;
		}
		else
		if( name == "dbcolb" )
		{
			_ColSelB.link( value.c_str() );
			return;
		}
		else
		if( name == "dbcola" )
		{
			_ColSelA.link( value.c_str() );
			return;
		}
		else
			CCtrlBase::setProperty( name, value );
	}

	xmlNodePtr CCtrlColPick::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CCtrlBase::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "colpick" );

		xmlSetProp( node, BAD_CAST "texture",
			BAD_CAST CViewRenderer::getInstance()->getTextureNameFromId( _Texture ).c_str() );

		xmlSetProp( node, BAD_CAST "onchange", BAD_CAST _AHOnChange.c_str() );
		xmlSetProp( node, BAD_CAST "onchange_params", BAD_CAST _AHOnChangeParams.c_str() );

		std::string s;
		
		if( _ColSelR.getNodePtr() != NULL )
			s = _ColSelR.getNodePtr()->getFullName();
		else
			s = "";
		xmlSetProp( node, BAD_CAST "dbcolr", BAD_CAST s.c_str() );

		if( _ColSelG.getNodePtr() != NULL )
			s = _ColSelG.getNodePtr()->getFullName();
		else
			s = "";
		xmlSetProp( node, BAD_CAST "dbcolg", BAD_CAST s.c_str() );

		if( _ColSelB.getNodePtr() != NULL )
			s = _ColSelB.getNodePtr()->getFullName();
		else
			s = "";
		xmlSetProp( node, BAD_CAST "dbcolb", BAD_CAST s.c_str() );

		if( _ColSelA.getNodePtr() != NULL )
			s = _ColSelA.getNodePtr()->getFullName();
		else
			s = "";
		xmlSetProp( node, BAD_CAST "dbcola", BAD_CAST s.c_str() );

		return node;
	}

	// ------------------------------------------------------------------------------------------------
	bool CCtrlColPick::parse(xmlNodePtr node, CInterfaceGroup * parentGroup)
	{
		if (!CCtrlBase::parse(node, parentGroup))
			return false;

		CXMLAutoPtr prop;
		// Read textures
		prop = (char*) xmlGetProp( node, (xmlChar*)"texture" );
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		if(prop)
		{
			string sTmp = NLMISC::toLower((const char*)prop);
			_Texture = rVR.createTexture (sTmp, 0, 0, 256, 64, false, false);
		}

		prop = (char*) xmlGetProp( node, (xmlChar*)"onchange" );
		if (prop)	_AHOnChange = NLMISC::toLower((const char*)prop);
		prop = (char*) xmlGetProp( node, (xmlChar*)"onchange_params" );
		if (prop)	_AHOnChangeParams = string((const char*)prop);

		prop = (char*) xmlGetProp( node, (xmlChar*)"dbcolr" );
		if (prop) _ColSelR.link(prop);
		prop = (char*) xmlGetProp( node, (xmlChar*)"dbcolg" );
		if (prop) _ColSelG.link(prop);
		prop = (char*) xmlGetProp( node, (xmlChar*)"dbcolb" );
		if (prop) _ColSelB.link(prop);
		prop = (char*) xmlGetProp( node, (xmlChar*)"dbcola" );
		if (prop) _ColSelA.link(prop);

		return true;
	}

	// ------------------------------------------------------------------------------------------------
	void CCtrlColPick::updateCoords()
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		sint32 txw, txh;
		rVR.getTextureSizeFromId (_Texture, txw, txh);
		_W = txw;
		_H = txh;
		CCtrlBase::updateCoords();
	}

	// ------------------------------------------------------------------------------------------------
	void CCtrlColPick::draw()
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();

		CRGBA col = CRGBA(255,255,255,(uint8)CWidgetManager::getInstance()->getGlobalColor().A);

		rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal,
								_WReal, _HReal,
								0, false,
								_Texture,
								col );
	}

	// ------------------------------------------------------------------------------------------------
	bool CCtrlColPick::handleEvent (const NLGUI::CEventDescriptor &event)
	{
		if (CCtrlBase::handleEvent(event)) return true;
		if (!_Active)
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
				selectColor(eventDesc.getX()-_XReal, eventDesc.getY()-_YReal);
				return true;
			}
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup)
			{
				_MouseDown = false;
				return true;
			}
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousemove)
			{
				if (_MouseDown)
				{
					selectColor(eventDesc.getX()-_XReal, eventDesc.getY()-_YReal);
				}
				return true;
			}
		}
		return false;
	}

	// ------------------------------------------------------------------------------------------------
	string CCtrlColPick::getColor () const
	{
		return NLMISC::toString(_ColorSelect.R) + " " + NLMISC::toString(_ColorSelect.G) + " " + NLMISC::toString(_ColorSelect.B) + " " + NLMISC::toString(_ColorSelect.A);
	}

	// ------------------------------------------------------------------------------------------------
	void CCtrlColPick::setColor (const string &col)
	{
		_ColorSelect = convertColor (col.c_str());
	}

	// ------------------------------------------------------------------------------------------------
	string CCtrlColPick::getColorOver () const
	{
		return NLMISC::toString(_ColorOver.R) + " " + NLMISC::toString(_ColorOver.G) + " " + NLMISC::toString(_ColorOver.B) + " " + NLMISC::toString(_ColorOver.A);
	}

	// ------------------------------------------------------------------------------------------------
	void CCtrlColPick::setColorOver (const string &col)
	{
		_ColorOver = convertColor (col.c_str());
	}

	// ------------------------------------------------------------------------------------------------
	void CCtrlColPick::selectColor (sint32 x, sint32 y)
	{

		_ColorSelect = getColor (x, y);

		if (_ColSelR.getNodePtr() != NULL)
			_ColSelR.setSInt32(_ColorSelect.R);
		if (_ColSelG.getNodePtr() != NULL)
			_ColSelG.setSInt32(_ColorSelect.G);
		if (_ColSelB.getNodePtr() != NULL)
			_ColSelB.setSInt32(_ColorSelect.B);
		if (_ColSelA.getNodePtr() != NULL)
			_ColSelA.setSInt32(_ColorSelect.A);

		if (!_AHOnChange.empty())
			CAHManager::getInstance()->runActionHandler(_AHOnChange, this, _AHOnChangeParams);
	}

	// ------------------------------------------------------------------------------------------------
	CRGBA CCtrlColPick::getColor (sint32 x, sint32 y)
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();

		if (x < 0) x = 0;
		if (y < 0) y = 0;
		if (x >= _WReal) x = _WReal-1;
		if (y >= _HReal) y = _HReal-1;
		return rVR.getTextureColor (_Texture, x, y);
	}

}


