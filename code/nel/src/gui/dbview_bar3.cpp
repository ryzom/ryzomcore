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


#include "nel/gui/dbview_bar3.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/db_manager.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;

namespace NLGUI
{
	NLMISC_REGISTER_OBJECT(CViewBase, CDBViewBar3, std::string, "bar3");

	// ----------------------------------------------------------------------------
	CDBViewBar3::CDBViewBar3(const TCtorParam &param)
	:	CViewBitmap(param),
		_Slot(TCtorParam())
	{
		_Mini = false;
		_ColorsNegative[0] = _ColorsNegative[1] = _ColorsNegative[2] = NLMISC::CRGBA(0,0,0,0);
		_ValueInt[0] = _ValueInt[1] = _ValueInt[2] = 0;
		_RangeInt[0] = _RangeInt[1] = _RangeInt[2] = 255;
	}


	// ----------------------------------------------------------------------------
	void CDBViewBar3::parseValProp(xmlNodePtr cur, CInterfaceProperty &dbProp, sint32 &intProp, const char *name)
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


	std::string CDBViewBar3::getProperty( const std::string &name ) const
	{
		if( name == "value1" )
		{
			return getValProp( _Value[ 0 ], _ValueInt[ 0 ] );
		}
		else
		if( name == "value2" )
		{
			return getValProp( _Value[ 1 ], _ValueInt[ 1 ] );
		}
		else
		if( name == "value3" )
		{
			return getValProp( _Value[ 2 ], _ValueInt[ 2 ] );
		}
		else
		if( name == "range1" )
		{
			return getValProp( _Range[ 0 ], _RangeInt[ 0 ] );
		}
		else
		if( name == "range2" )
		{
			return getValProp( _Range[ 1 ], _RangeInt[ 1 ] );
		}
		else
		if( name == "range3" )
		{
			return getValProp( _Range[ 2 ], _RangeInt[ 2 ] );
		}
		else
		if( name == "color1" )
		{
			return toString( _Colors[ 0 ] );
		}
		else
		if( name == "color2" )
		{
			return toString( _Colors[ 1 ] );
		}
		else
		if( name == "color3" )
		{
			return toString( _Colors[ 2 ] );
		}
		else
		if( name == "color1_negative" )
		{
			return toString( _ColorsNegative[ 0 ] );
		}
		else
		if( name == "color2_negative" )
		{
			return toString( _ColorsNegative[ 1 ] );
		}
		else
		if( name == "color3_negative" )
		{
			return toString( _ColorsNegative[ 2 ] );
		}
		else
		if( name == "mini" )
		{
			if( _Mini )
				return "true";
			else
				return "false";
		}
		else
			return CViewBitmap::getProperty( name );
	}


	// ----------------------------------------------------------------------------
	bool CDBViewBar3::parse (xmlNodePtr cur, CInterfaceGroup * parentGroup)
	{
		if (!CViewBitmap::parse(cur, parentGroup))
		{
			string tmp = "cannot parse view:"+getId()+", parent:"+parentGroup->getId();
			nlinfo(tmp.c_str());
			return false;
		}

		// read values
		parseValProp(cur, _Value[0], _ValueInt[0], "value1" );
		parseValProp(cur, _Value[1], _ValueInt[1], "value2" );
		parseValProp(cur, _Value[2], _ValueInt[2], "value3" );

		// read ranges
		parseValProp(cur, _Range[0], _RangeInt[0], "range1" );
		parseValProp(cur, _Range[1], _RangeInt[1], "range2" );
		parseValProp(cur, _Range[2], _RangeInt[2], "range3" );


		// Read colors etc....
		CXMLAutoPtr prop;

		prop = (char*) xmlGetProp( cur, (xmlChar*)"color1" );
		_Colors[0] = convertColor(prop);
		prop = (char*) xmlGetProp( cur, (xmlChar*)"color2" );
		_Colors[1] = convertColor(prop);
		prop = (char*) xmlGetProp( cur, (xmlChar*)"color3" );
		_Colors[2] = convertColor(prop);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"color1_negative" );
		_ColorsNegative[0] = convertColor(prop);
		prop = (char*) xmlGetProp( cur, (xmlChar*)"color2_negative" );
		_ColorsNegative[1] = convertColor(prop);
		prop = (char*) xmlGetProp( cur, (xmlChar*)"color3_negative" );
		_ColorsNegative[2] = convertColor(prop);


		_Mini = false;
		prop = (char*) xmlGetProp( cur, (xmlChar*)"mini" );
		if (prop)
			if (convertBool(prop))
				setMini(true);

		if (_Mini == false)
			setMini(false);

		return true;
	}

	// ----------------------------------------------------------------------------
	void CDBViewBar3::setMini(bool b)
	{
		_Mini = b;
		if (_Mini)
			_Slot.setTexture ("w_slot_jauge_3_mini.tga");
		else
			_Slot.setTexture ("w_slot_jauge_3.tga");

		_Slot.setPosRef (_PosRef);
		_Slot.setParentPosRef (_ParentPosRef);
		_Slot.setX (_X);
		_Slot.setY (_Y);

		_Scale = true;
		if (_Mini)
			setTexture ("w_jauge_fill_mini.tga");
		else
			setTexture ("w_jauge_fill.tga");
	}

	// ----------------------------------------------------------------------------
	void CDBViewBar3::updateCoords ()
	{
		if (_ParentPos == NULL)
			_Slot.setParentPos (_Parent);
		else
			_Slot.setParentPos (_ParentPos);
		_Slot.updateCoords();
		_W = _Slot.getW();
		_H = _Slot.getH();

		// get the height of the texture
		sint32	dummy;
		CViewRenderer::getInstance()->getTextureSizeFromId(_TextureId, dummy, _BarH);

		CViewBitmap::updateCoords();
	}

	// ----------------------------------------------------------------------------
	sint32	CDBViewBar3::getCurrentValProp(const CInterfaceProperty &dbProp, sint32 intProp)
	{
		if(dbProp.getNodePtr())
			return dbProp.getSInt32();
		else
			return intProp;
	}

	std::string CDBViewBar3::getValProp( const CInterfaceProperty &prop, sint32 intProp ) const
	{
		if( prop.getNodePtr() != NULL )
			return prop.getNodePtr()->getFullName();
		else
			return toString( intProp );
	}


	// ----------------------------------------------------------------------------
	void CDBViewBar3::draw ()
	{
		_Slot.draw();

		CViewRenderer &rVR = *CViewRenderer::getInstance();
		CRGBA gColor = CWidgetManager::getInstance()->getGlobalColorForContent();

		if (_Mini)
		{
			for (uint32 i = 0; i < 3; ++i)
			{
				float	factor;
				CRGBA color;

				sint32	value= getCurrentValProp(_Value[i], _ValueInt[i]);
				sint32	range= getCurrentValProp(_Range[i], _RangeInt[i]);

				if (range > 0)
					factor = ( (float)value / (float)range );
				else
					factor = 0;

				if (factor < 0)
				{
					factor = -factor;
					color = _ColorsNegative[i];
				}
				else
				{
					color = _Colors[i];
				}

				// clamp the factor to 0/1
				clamp(factor, 0, 1);
				float wBar= factor * (float)(_Slot.getWReal()-2);

				color.A = (uint8)(((sint32)color.A*((sint32)gColor.A+1))>>8);
				_WReal = (sint32)wBar;

				rVR.drawRotFlipBitmap (_RenderLayer, _XReal+1, _YReal+i*3+2, _WReal, _BarH, 0, false, _TextureId, color);
			}
		}
		else
		{
			for (uint32 i = 0; i < 3; ++i)
			{
				float factor;
				CRGBA color;

				sint32	value= getCurrentValProp(_Value[i], _ValueInt[i]);
				sint32	range= getCurrentValProp(_Range[i], _RangeInt[i]);

				if (range > 0)
					factor = ( (float)value / (float)range );
				else
					factor = 0;

				if (factor < 0)
				{
					factor = -factor;
					color = _ColorsNegative[i];
				}
				else
				{
					color = _Colors[i];
				}

				// clamp the factor to 0/1
				clamp(factor, 0, 1);
				float wBar= factor * (float)(_Slot.getWReal()-4);

				color.A = (uint8)(((sint32)color.A*((sint32)gColor.A+1))>>8);
				_WReal = (sint32)wBar;

				rVR.drawRotFlipBitmap (_RenderLayer, _XReal+2, _YReal+i*7+4, _WReal, _BarH, 0, false, _TextureId, color);
			}
		}
	}

	void CDBViewBar3::forceLink()
	{
	}
}

