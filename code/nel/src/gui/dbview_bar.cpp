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
#include "nel/gui/dbview_bar.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/db_manager.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;

NLMISC_REGISTER_OBJECT(CViewBase, CDBViewBar, std::string, "bar");

namespace NLGUI
{

	// ----------------------------------------------------------------------------
	void CDBViewBar::parseValProp(xmlNodePtr cur, CInterfaceProperty &dbProp, sint32 &intProp, const char *name)
	{
		CXMLAutoPtr prop((const char*) xmlGetProp( cur, (xmlChar*)name ));
		if (prop)
		{
			if ( isdigit(*prop.getDatas()) || *(prop.getDatas())=='-')
				fromString((const char*)prop, intProp);
			else
				dbProp.link(prop);
		}
	}

	std::string CDBViewBar::getProperty( const std::string &name ) const
	{
		if( name == "value" )
		{
			if( _Value.getNodePtr() != NULL )
				return _Value.getNodePtr()->getFullName();
			else
				return toString( _ValueInt );
		}
		else
		if( name == "range" )
		{
			if( _Range.getNodePtr() != NULL )
				return _Range.getNodePtr()->getFullName();
			else
				return toString( _RangeInt );
		}
		else
		if( name == "reference" )
		{
			if( _Reference.getNodePtr() != NULL )
				return _Reference.getNodePtr()->getFullName();
			else
				return toString( _ReferenceInt );
		}
		else
		if( name == "color_negative" )
		{
			return toString( _ColorNegative );
		}
		else
		if( name == "mini" )
		{
			if( _Type == ViewBar_Mini )
				return "true";
			else
				return "false";
		}
		else
		if( name == "ultra_mini" )
		{
			if( _Type == ViewBar_UltraMini )
				return "true";
			else
				return "false";
		}
		else
		if( name == "mini_thick" )
		{
			if( _Type == ViewBar_MiniThick )
				return "true";
			else
				return "false";
		}
		else
			return CViewBitmap::getProperty( name );
	}

	void CDBViewBar::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "value" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_ValueInt = i;
			else
				_Value.link( value.c_str() );
			return;
		}
		else
		if( name == "range" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_RangeInt = i;
			else
				_Range.link( value.c_str() );
			return;
		}
		else
		if( name == "reference" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_ReferenceInt = i;
			else
				_Reference.link( value.c_str() );
			return;
		}
		else
		if( name == "color_negative" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				_ColorNegative = c;
			return;
		}
		else
		if( name == "mini" )
		{
			bool b;
			if( fromString( value, b ) )
				if( b )
					_Type = ViewBar_Mini;
			return;
		}
		else
		if( name == "ultra_mini" )
		{
			bool b;
			if( fromString( value, b ) )
				if( b )
					_Type = ViewBar_UltraMini;
			return;
		}
		else
		if( name == "mini_thick" )
		{
			bool b;
			if( fromString( value, b ) )
				if( b )
					_Type = ViewBar_MiniThick;
			return;
		}
		else
			CViewBitmap::setProperty( name, value );
	}


	xmlNodePtr CDBViewBar::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CViewBitmap::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "bar" );

		if( _Value.getNodePtr() != NULL )
			xmlSetProp( node, BAD_CAST "value", BAD_CAST _Value.getNodePtr()->getFullName().c_str() );
		else
			xmlSetProp( node, BAD_CAST "value", BAD_CAST toString( _RangeInt ).c_str() );

		if( _Range.getNodePtr() != NULL )
			xmlSetProp( node, BAD_CAST "range", BAD_CAST _Range.getNodePtr()->getFullName().c_str() );
		else
			xmlSetProp( node, BAD_CAST "range", BAD_CAST toString( _RangeInt ).c_str() );

		if( _Reference.getNodePtr() != NULL )
			xmlSetProp( node, BAD_CAST "reference", BAD_CAST _Reference.getNodePtr()->getFullName().c_str() );
		else
			xmlSetProp( node, BAD_CAST "reference", BAD_CAST toString( _ReferenceInt ).c_str() );

		xmlSetProp( node, BAD_CAST "color_negative", BAD_CAST toString( _ColorNegative ).c_str() );
		
		if( _Type == ViewBar_Mini )
			xmlSetProp( node, BAD_CAST "mini", BAD_CAST "true" );
		else
			xmlSetProp( node, BAD_CAST "mini", BAD_CAST "false" );

		if( _Type == ViewBar_UltraMini )
			xmlSetProp( node, BAD_CAST "ultra_mini", BAD_CAST "true" );
		else
			xmlSetProp( node, BAD_CAST "ultra_mini", BAD_CAST "false" );

		if( _Type == ViewBar_MiniThick )
			xmlSetProp( node, BAD_CAST "mini_thick", BAD_CAST "true" );
		else
			xmlSetProp( node, BAD_CAST "mini_thick", BAD_CAST "false" );

		return node;
	}

	// ----------------------------------------------------------------------------
	bool CDBViewBar::parse (xmlNodePtr cur, CInterfaceGroup * parentGroup)
	{
		if (!CViewBitmap::parse(cur, parentGroup))
		{
			string tmp = "cannot parse view:"+getId()+", parent:"+parentGroup->getId();
			nlinfo(tmp.c_str());
			return false;
		}

		CXMLAutoPtr prop;

		// read value, range and reference
		parseValProp(cur, _Value, _ValueInt, "value");
		parseValProp(cur, _Range, _RangeInt, "range");
		parseValProp(cur, _Reference, _ReferenceInt, "reference");

		// Get Visual props
		prop= (char*) xmlGetProp( cur, (xmlChar*)"color_negative" );
		_ColorNegative = CRGBA(0,0,0,0);
		if (prop)
			_ColorNegative = convertColor (prop);

		// Bar Type
		_Type = ViewBar_Normal;

		prop = (char*) xmlGetProp( cur, (xmlChar*)"mini" );
		if (prop)
			if (convertBool(prop))
				setType(ViewBar_Mini);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"ultra_mini" );
		if (prop)
			if (convertBool(prop))
				setType(ViewBar_UltraMini);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"mini_thick" );
		if (prop)
			if (convertBool(prop))
				setType(ViewBar_MiniThick);

		if (_Type == ViewBar_Normal)
			setType(ViewBar_Normal);

		return true;
	}

	// ----------------------------------------------------------------------------
	void CDBViewBar::setType (TViewBar vb)
	{
		_Type = vb;
		switch(_Type)
		{
			case ViewBar_Normal:	_Slot.setTexture ("w_slot_jauge_1.tga"); break;
			case ViewBar_Mini:		_Slot.setTexture ("w_slot_jauge_1_mini.tga"); break;
			case ViewBar_UltraMini: _Slot.setTexture ("w_slot_jauge_1_umin.tga"); break;
			case ViewBar_MiniThick: _Slot.setTexture ("w_slot_jauge_1_tmin.tga"); break;
		}

		_Slot.setPosRef (_PosRef);
		_Slot.setParentPosRef (_ParentPosRef);
		_Slot.setX (_X);
		_Slot.setY (_Y);

		_Scale = true;
		switch(_Type)
		{
			case ViewBar_Normal:	setTexture ("w_jauge_fill.tga"); break;
			case ViewBar_Mini:		setTexture ("w_jauge_fill_mini.tga"); break;
			case ViewBar_UltraMini: setTexture ("w_jauge_fill_umin.tga"); break;
			case ViewBar_MiniThick: setTexture ("w_jauge_fill_tmin.tga"); break;
		}

		// Get the Height Size.
		sint32	wBar;
		CViewRenderer::getInstance()->getTextureSizeFromId(_TextureId, wBar, _HBar);
	}

	// ----------------------------------------------------------------------------
	void CDBViewBar::updateCoords ()
	{
		if (_ParentPos == NULL)
			_Slot.setParentPos (_Parent);
		else
			_Slot.setParentPos (_ParentPos);
		_Slot.updateCoords();
		_W = _Slot.getW();
		_H = _Slot.getH();
		CViewBitmap::updateCoords();
	}

	// ----------------------------------------------------------------------------
	sint64	CDBViewBar::getCurrentValProp(const CInterfaceProperty &dbProp, sint32 intProp)
	{
		if(dbProp.getNodePtr())
			return dbProp.getSInt64();
		else
			return intProp;
	}

	// ----------------------------------------------------------------------------
	void CDBViewBar::draw ()
	{
		float wBar = (float)(_Slot.getWReal()-4);

		sint64	value= getCurrentValProp(_Value, _ValueInt);
		sint64	range= getCurrentValProp(_Range, _RangeInt);
		sint64	reference= getCurrentValProp(_Reference, _ReferenceInt);

		// remove the reference
		value-= reference;
		range-= reference;

		// draw the bar
		CRGBA color = _Color;

		if (range > 0)
		{
			float	ratio= (float)value / range;
			if (_ColorNegative.A != 0 && ratio < 0.0f)
			{
				ratio = - ratio;
				color = _ColorNegative;
			}
			NLMISC::clamp(ratio, 0.f, 1.f);
			wBar *= ratio;
		}
		else
			wBar = 0;

		_WReal = (sint32)wBar;

		_Slot.draw();

		CViewRenderer &rVR = *CViewRenderer::getInstance();

		color.A = (uint8)(((sint32)color.A*((sint32)CWidgetManager::getInstance()->getGlobalColorForContent().A+1))>>8);

		// compute the DeltaY: mean of dif.
		sint32	deltaY= (_H-_HBar)/2;
		rVR.drawRotFlipBitmap (_RenderLayer, _XReal+2, _YReal+deltaY, _WReal, _HBar, 0, false, _TextureId, color);
	}

	// ----------------------------------------------------------------------------
	void CDBViewBar::setValueDbLink (const std::string &r)
	{
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(r,false);
		if (pNL != NULL) _Value.setNodePtr(pNL);
	}

	// ----------------------------------------------------------------------------
	void CDBViewBar::setRangeDbLink (const std::string &r)
	{
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(r,false);
		if (pNL != NULL) _Range.setNodePtr(pNL);
	}

	// ----------------------------------------------------------------------------
	void CDBViewBar::setReferenceDbLink (const std::string &r)
	{
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(r,false);
		if (pNL != NULL) _Reference.setNodePtr(pNL);
	}

	// ----------------------------------------------------------------------------
	string CDBViewBar::getValueDbLink () const
	{
		if (_Value.getNodePtr() == NULL) return "";
		return _Value.getNodePtr()->getFullName();
	}

	// ----------------------------------------------------------------------------
	string CDBViewBar::getRangeDbLink () const
	{
		if (_Range.getNodePtr() == NULL) return "";
		return _Range.getNodePtr()->getFullName();
	}

	// ----------------------------------------------------------------------------
	string CDBViewBar::getReferenceDbLink () const
	{
		if (_Reference.getNodePtr() == NULL) return "";
		return _Reference.getNodePtr()->getFullName();
	}

}

