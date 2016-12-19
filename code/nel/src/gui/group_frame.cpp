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
#include "nel/gui/group_frame.h"

#include "nel/gui/widget_manager.h"
#include "nel/gui/interface_options.h"
#include "nel/gui/interface_element.h"
#include "nel/gui/view_renderer.h"
#include "nel/misc/xml_auto_ptr.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{

	NLMISC_REGISTER_OBJECT(CViewBase, CGroupFrame, std::string, "frame");

	// ***************************************************************************
	vector<CGroupFrame::SDisplayType> CGroupFrame::_DispTypes;

	// ***************************************************************************
	CGroupFrame::CGroupFrame(const TCtorParam &param)
	: CInterfaceGroup(param)
	{
		_DisplayFrame = true;
		_Color = CRGBA(255,255,255,255);
		_DispType = 0;
		_DisplayFrameDefined= false;
		_ColorDefined= false;
		_DispTypeDefined= false;
	}

	std::string CGroupFrame::getProperty( const std::string &name ) const
	{
		if( name == "display" )
		{
			return toString( _DisplayFrame );
		}
		else
		if( name == "color" )
		{
			return toString( _Color );
		}
		else
		if( name == "options" )
		{
			return _Options;
		}
		else
			return CInterfaceGroup::getProperty( name );
	}

	void CGroupFrame::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "display" )
		{
			bool b;
			if( fromString( value, b ) )
				_DisplayFrame = b;
			return;
		}
		else
		if( name == "color" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				_Color = c;
			return;
		}
		else
		if( name == "options" )
		{
			_Options = value;
			setupOptions();
			return;
		}
		else
			CInterfaceGroup::setProperty( name, value );
	}

	xmlNodePtr CGroupFrame::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CInterfaceGroup::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "frame" );
		xmlSetProp( node, BAD_CAST "display", BAD_CAST NLMISC::toString( _DisplayFrame ).c_str() );
		xmlSetProp( node, BAD_CAST "color", BAD_CAST NLMISC::toString( _Color ).c_str() );
		xmlSetProp( node, BAD_CAST "options", BAD_CAST _Options.c_str() );

		return node;
	}

	// ***************************************************************************
	bool CGroupFrame::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
	{
		if(!CInterfaceGroup::parse(cur, parentGroup))
			return false;

		// display
		CXMLAutoPtr ptr((const char*) xmlGetProp (cur, (xmlChar*)"display"));
		_DisplayFrame = true;
		_DisplayFrameDefined= false;
		if (ptr)
		{
			_DisplayFrame = convertBool (ptr);
			_DisplayFrameDefined= true;
		}

		// color
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"color" );
		_Color = CRGBA(255,255,255,255);
		_ColorDefined= false;
		if (ptr)
		{
			_Color = convertColor (ptr);
			_ColorDefined= true;
		}

		// Get the borders texture
		_DispTypeDefined= false;
		CViewRenderer &rVR = *CViewRenderer::getInstance();

		ptr = (char*) xmlGetProp( cur, (xmlChar*)"options" );
		if (ptr)
		{
			_Options = std::string( (const char*)ptr );
		}

		// The first type in display type struct is the default display type
		if (_DispTypes.empty())
		{
			SDisplayType dt;
			dt.Name = "default";
			// get texture ids.
			dt.BorderIds[TextTL]= rVR.getTextureIdFromName ("w_modal_tl.tga");
			dt.BorderIds[TextTM]= rVR.getTextureIdFromName ("w_modal_t.tga");
			dt.BorderIds[TextTR]= rVR.getTextureIdFromName ("w_modal_tr.tga");
			// middle
			dt.BorderIds[TextML]= rVR.getTextureIdFromName ("w_modal_l.tga");
			dt.BorderIds[TextMM]= rVR.getTextureIdFromName ("w_modal_blank.tga");
			dt.BorderIds[TextMR]= rVR.getTextureIdFromName ("w_modal_r.tga");
			// bottom
			dt.BorderIds[TextBL]= rVR.getTextureIdFromName ("w_modal_bl.tga");
			dt.BorderIds[TextBM]= rVR.getTextureIdFromName ("w_modal_b.tga");
			dt.BorderIds[TextBR]= rVR.getTextureIdFromName ("w_modal_br.tga");
			// get size
			rVR.getTextureSizeFromId (dt.BorderIds[TextTL], dt.LeftBorder, dt.TopBorder);
			rVR.getTextureSizeFromId (dt.BorderIds[TextBR], dt.RightBorder, dt.BottomBorder);
			_DispTypes.push_back(dt);
		}

		setupOptions();

		return true;
	}


	// ***************************************************************************
	void CGroupFrame::draw ()
	{
		if (_DisplayFrame)
		{
			CViewRenderer &rVR = *CViewRenderer::getInstance();

			// get global color
			CRGBA col;
			if(getModulateGlobalColor())
				col.modulateFromColor( _Color, CWidgetManager::getInstance()->getGlobalColor() );
			else
				col= _Color;

			// draw the background
			sint xId = 0, yId = 0;
			for (yId = 0; yId < 3; yId++)
			{
				for (xId = 0; xId < 3; xId++)
				{
					sint32	x = _XReal;
					sint32	y = _YReal;
					sint32	w, h;
					// top
					if (yId == 0)
					{
						y += _HReal-_DispTypes[_DispType].TopBorder;
						h = _DispTypes[_DispType].TopBorder;
					}
					// Middle
					else if (yId == 1)
					{
						y += _DispTypes[_DispType].BottomBorder;
						h = _HReal-_DispTypes[_DispType].TopBorder-_DispTypes[_DispType].BottomBorder;
					}
					// Bottom
					else
					{
						h = _DispTypes[_DispType].BottomBorder;
					}

					// Left
					if (xId == 0)
					{
						w = _DispTypes[_DispType].LeftBorder;
					}
					else if (xId == 1)
					{
						x += _DispTypes[_DispType].LeftBorder;
						w = _WReal-_DispTypes[_DispType].LeftBorder-_DispTypes[_DispType].RightBorder;
					}
					else
					{
						x += _WReal-_DispTypes[_DispType].RightBorder;
						w = _DispTypes[_DispType].RightBorder;
					}

					// render
					uint8 tile = _DispTypes[_DispType].TileBorder[yId*3+xId];
					if (tile == 0)
						rVR.drawRotFlipBitmap (_RenderLayer, x, y, w, h, 0, false, _DispTypes[_DispType].BorderIds[yId*3+xId], col);
					else
						rVR.drawRotFlipBitmapTiled (_RenderLayer, x, y, w, h, 0, false, _DispTypes[_DispType].BorderIds[yId*3+xId], tile-1, col);

				}
			}
		}
		// draw the components
		CInterfaceGroup::draw();
	}

	// ***************************************************************************
	void CGroupFrame::copyOptionFrom(const CGroupFrame &other)
	{
		CInterfaceGroup::copyOptionFrom(other);

		// Copy option only if they were not explicitly defined in xml
		if(!_DisplayFrameDefined)
			_DisplayFrame = other._DisplayFrame;
		if(!_ColorDefined)
			_Color = other._Color;
		if(!_DispTypeDefined)
			_DispType = other._DispType;
	}

	void CGroupFrame::setupOptions()
	{
		CViewRenderer &rVR = *(CViewRenderer::getInstance());

		CInterfaceOptions *pIO = NULL;
		pIO = CWidgetManager::getInstance()->getOptions( _Options );

		if( pIO != NULL )
		{
			_DispTypeDefined= true;

			// Look if we find the type...
			uint32 i;
			for (i = 0; i < _DispTypes.size(); ++i)
				if (_DispTypes[i].Name == _Options )
					break;

			if (i == _DispTypes.size())
			{
				SDisplayType dt;
				dt.Name = _Options;
				// get texture ids.
				dt.BorderIds[TextTL]= rVR.getTextureIdFromName (pIO->getValStr("tx_tl"));
				dt.BorderIds[TextTM]= rVR.getTextureIdFromName (pIO->getValStr("tx_t"));
				dt.BorderIds[TextTR]= rVR.getTextureIdFromName (pIO->getValStr("tx_tr"));
				// middle
				dt.BorderIds[TextML]= rVR.getTextureIdFromName (pIO->getValStr("tx_l"));
				dt.BorderIds[TextMM]= rVR.getTextureIdFromName (pIO->getValStr("tx_blank"));
				dt.BorderIds[TextMR]= rVR.getTextureIdFromName (pIO->getValStr("tx_r"));
				// bottom
				dt.BorderIds[TextBL]= rVR.getTextureIdFromName (pIO->getValStr("tx_bl"));
				dt.BorderIds[TextBM]= rVR.getTextureIdFromName (pIO->getValStr("tx_b"));
				dt.BorderIds[TextBR]= rVR.getTextureIdFromName (pIO->getValStr("tx_br"));

				// Tile
				dt.TileBorder[TextTM] = (uint8)pIO->getValSInt32("tile_t");
				dt.TileBorder[TextML] = (uint8)pIO->getValSInt32("tile_l");
				dt.TileBorder[TextMM] = (uint8)pIO->getValSInt32("tile_blank");
				dt.TileBorder[TextMR] = (uint8)pIO->getValSInt32("tile_r");
				dt.TileBorder[TextBM] = (uint8)pIO->getValSInt32("tile_b");

				// get size
				rVR.getTextureSizeFromId (dt.BorderIds[TextTL], dt.LeftBorder, dt.TopBorder);
				rVR.getTextureSizeFromId (dt.BorderIds[TextBR], dt.RightBorder, dt.BottomBorder);
				_DispTypes.push_back(dt);
			}
			_DispType = (uint8)i;
		}
		else
		{
			_DispType = 0;
		}
	}

	// ***************************************************************************
	void CGroupFrame::setColorAsString(const string & col)
	{
		_Color = convertColor (col.c_str());
	}

	// ***************************************************************************
	string CGroupFrame::getColorAsString() const
	{
		return NLMISC::toString(_Color.R) + " " + NLMISC::toString(_Color.G) + " " + NLMISC::toString(_Color.B) + " " + NLMISC::toString(_Color.A);
	}

}

