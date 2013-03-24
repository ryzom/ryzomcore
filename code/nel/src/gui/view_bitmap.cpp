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
#include "nel/gui/view_bitmap.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/group_container_base.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;


NLMISC_REGISTER_OBJECT(CViewBase, CViewBitmap, std::string, "bitmap");
REGISTER_UI_CLASS(CViewBitmap)

namespace NLGUI
{

	std::string CViewBitmap::getProperty( const std::string &name ) const
	{
		if( name == "color" )
		{
			return toString( _Color );
		}
		else
		if( name == "txtoffsetx" )
		{
			return toString( _TxtOffsetX );
		}
		else
		if( name == "txtoffsety" )
		{
			return toString( _TxtOffsetY );
		}
		else
		if( name == "txtwidth" )
		{
			return toString( _TxtWidth );
		}
		else
		if( name == "txtheight" )
		{
			return toString( _TxtHeight );
		}
		else
		if( name == "texture" )
		{
			return getTexture();
		}
		else
		if( name == "scale" )
		{
			return toString( _Scale );
		}
		else
		if( name == "rot" )
		{
			return toString( _Rot );
		}
		else
		if( name == "flip" )
		{
			return toString( _Flip );
		}
		else
		if( name == "tile" )
		{
			return toString( _Tile );
		}
		else
		if( name == "align" )
		{
			std::string align;

			if( ( _Align & 1 ) != 0 )
				align += "R";
			else
				align += "L";

			if( ( _Align & 2 ) != 0 )
				align += "T";
			else
				align += "B";

			return align;
		}
		else
		if( name == "inherit_gc_alpha" )
		{
			return toString( _InheritGCAlpha );
		}
		else
			return CViewBase::getProperty( name );
	}

	void CViewBitmap::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "color" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				_Color = c;
			return;
		}
		else
		if( name == "txtoffsetx" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_TxtOffsetX = i;
			return;
		}
		else
		if( name == "txtoffsety" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_TxtOffsetY = i;
			return;
		}
		else
		if( name == "txtwidth" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_TxtWidth = i;
			return;
		}
		else
		if( name == "txtheight" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_TxtHeight = i;
			return;
		}
		else
		if( name == "texture" )
		{
			setTexture( value );
			return;
		}
		else
		if( name == "scale" )
		{
			bool b;
			if( fromString( value, b ) )
				_Scale = b;
			return;
		}
		else
		if( name == "rot" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_Rot = i;
			return;
		}
		else
		if( name == "flip" )
		{
			bool b;
			if( fromString( value, b ) )
				_Flip = b;
			return;
		}
		else
		if( name == "tile" )
		{
			bool b;
			if( fromString( value, b ) )
				_Tile = b;
			return;
		}
		else
		if( name == "align" )
		{
			std::string::size_type i;
			for( i = 0; i < value.size(); i++ )
			{
				const char c = value[ i ];

				switch( c )
				{
				case 'L':
					_Align &= ~1;
					break;

				case 'R':
					_Align |= 1;
					break;

				case 'B':
					_Align &= ~2;
					break;

				case 'T':
					_Align |= 2;
					break;
				}
			}
			return;
		}
		else
		if( name == "inherit_gc_alpha" )
		{
			bool b;
			if( fromString( value, b ) )
				_InheritGCAlpha = b;
			return;
		}
		else
			CViewBase::setProperty( name, value );
	}


	xmlNodePtr CViewBitmap::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CViewBase::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "bitmap" );
		xmlSetProp( node, BAD_CAST "color", BAD_CAST toString( _Color ).c_str() );
		xmlSetProp( node, BAD_CAST "txtoffsetx", BAD_CAST toString( _TxtOffsetX ).c_str() );
		xmlSetProp( node, BAD_CAST "txtoffsety", BAD_CAST toString( _TxtOffsetY ).c_str() );
		xmlSetProp( node, BAD_CAST "txtwidth", BAD_CAST toString( _TxtWidth ).c_str() );
		xmlSetProp( node, BAD_CAST "txtheight", BAD_CAST toString( _TxtHeight ).c_str() );
		xmlSetProp( node, BAD_CAST "texture", BAD_CAST getTexture().c_str() );
		xmlSetProp( node, BAD_CAST "scale", BAD_CAST toString( _Scale ).c_str() );
		xmlSetProp( node, BAD_CAST "rot", BAD_CAST toString( _Rot ).c_str() );
		xmlSetProp( node, BAD_CAST "flip", BAD_CAST toString( _Flip ).c_str() );
		xmlSetProp( node, BAD_CAST "tile", BAD_CAST toString( _Tile ).c_str() );
		xmlSetProp( node, BAD_CAST "inherit_gc_alpha", BAD_CAST toString( _InheritGCAlpha ).c_str() );

		std::string align;
		if( ( _Align & 1 ) != 0 )
			align += "R";
		else
			align += "L";

		if( ( _Align & 2 ) != 0 )
			align += "T";
		else
			align += "B";

		xmlSetProp( node, BAD_CAST "txtoffsetx", BAD_CAST align.c_str() );

		return node;
	}

	// ----------------------------------------------------------------------------

	bool CViewBitmap::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
	{
		CXMLAutoPtr prop;

		//try to get props that can be inherited from groups
		//if a property is not defined, try to find it in the parent group.
		//if it is undefined, set it to zero
		if (! CViewBase::parse(cur,parentGroup) )
		{
			string tmp = string("cannot parse view:")+getId()+", parent:"+parentGroup->getId();
			nlinfo (tmp.c_str());
			return false;
		}

		//try to get the NEEDED specific props
		prop= (char*) xmlGetProp( cur, (xmlChar*)"color" );
		_Color = CRGBA(255,255,255,255);
		if (prop)
			_Color = convertColor (prop);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"txtoffsetx" );
		_TxtOffsetX = 0;
		if (prop) fromString((const char*)prop, _TxtOffsetX);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"txtoffsety" );
		_TxtOffsetY = 0;
		if (prop) fromString((const char*)prop, _TxtOffsetY);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"txtwidth" );
		_TxtWidth = -1;
		if (prop) fromString((const char*)prop, _TxtWidth);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"txtheight" );
		_TxtHeight = -1;
		if (prop) fromString((const char*)prop, _TxtHeight);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"texture" );
		if (prop)
		{
			string TxName = (const char *) prop;
			TxName = strlwr (TxName);
			setTexture (TxName);
			//CInterfaceManager *pIM = CInterfaceManager::getInstance();
			//CViewRenderer &rVR = *CViewRenderer::getInstance();
			//_TextureId = rVR.getTextureIdFromName (TxName);
		}

		prop = (char*) xmlGetProp( cur, (xmlChar*)"scale" );
		_Scale = false;
		if (prop)
			_Scale = convertBool(prop);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"rot" );
		_Rot = 0;
		if (prop)
			fromString((const char*)prop, _Rot);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"flip" );
		_Flip = false;
		if (prop)
			_Flip = convertBool(prop);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"tile" );
		_Tile = false;
		if (prop)
			_Tile = convertBool(prop);

		prop = (char*) xmlGetProp (cur, (xmlChar*)"align");
		_Align = 0;
		if (prop)
		{
			const char *seekPtr = prop.getDatas();
			while (*seekPtr != 0)
			{
				if ((*seekPtr=='l')||(*seekPtr=='L'))
				{
					_Align &= ~1;
				}
				if ((*seekPtr=='r')||(*seekPtr=='R'))
				{
					_Align |= 1;
				}
				if ((*seekPtr=='b')||(*seekPtr=='B'))
				{
					_Align &= ~2;
				}
				if ((*seekPtr=='t')||(*seekPtr=='T'))
				{
					_Align |= 2;
				}
				++seekPtr;
			}
		}

		_InheritGCAlpha = false;
		prop = (char*) xmlGetProp( cur, (xmlChar*)"inherit_gc_alpha" );
		if (prop)
		{
			_InheritGCAlpha = convertBool(prop);
		}

		return true;
	}

	// ----------------------------------------------------------------------------
	void CViewBitmap::draw ()
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();

		CRGBA col;
		if(getModulateGlobalColor())
		{
			col.modulateFromColor (_Color, CWidgetManager::getInstance()->getGlobalColorForContent());
		}
		else
		{
			col= _Color;
			col.A = (uint8)(((sint32)col.A*((sint32)CWidgetManager::getInstance()->getGlobalColorForContent().A+1))>>8);
		}

		if (_InheritGCAlpha)
		{
			// search a parent container
			CInterfaceGroup *gr = getParent();
			while (gr)
			{
				if (gr->isGroupContainer())
				{
					CGroupContainerBase *gc = static_cast<CGroupContainerBase*>(gr);
					col.A = (uint8)(((sint32)col.A*((sint32)gc->getCurrentContainerAlpha()+1))>>8);
					break;
				}
				gr = gr->getParent();
			}
		}

		if (_Scale && !_Tile)
		{
			rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal,
									_WReal, _HReal,
									(uint8)_Rot, _Flip,
									_TextureId,
									col );
		}
		else
		{
			if (!_Tile)
			{
				rVR.draw11RotFlipBitmap (_RenderLayer, _XReal, _YReal,
										(uint8)_Rot, _Flip,
										_TextureId,
										col);
			}
			else
			{
				rVR.drawRotFlipBitmapTiled(_RenderLayer, _XReal, _YReal,
										   _WReal, _HReal,
										   (uint8)_Rot, _Flip,
										   _TextureId,
										   _Align,
										   col);
			}
		}
	}

	// ----------------------------------------------------------------------------
	void CViewBitmap::updateCoords()
	{
		if (!_Scale)
		{
			CViewRenderer &rVR = *CViewRenderer::getInstance();
			sint32 txw, txh;
			rVR.getTextureSizeFromId (_TextureId, txw, txh);
			_W = txw;
			_H = txh;
		}
		CViewBase::updateCoords();
	}

	// ----------------------------------------------------------------------------
	void CViewBitmap::setTexture(const std::string & TxName)
	{

		_TextureId.setTexture (TxName.c_str (), _TxtOffsetX, _TxtOffsetY, _TxtWidth, _TxtHeight, false);
	}

	// ----------------------------------------------------------------------------
	std::string CViewBitmap::getTexture () const
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		return rVR.getTextureNameFromId (_TextureId);
	}

	// ***************************************************************************
	void CViewBitmap::fitTexture()
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		sint32 w, h;
		rVR.getTextureSizeFromId(_TextureId, w, h);
		setW(w);
		setH(h);
	}

	// ***************************************************************************
	void CViewBitmap::setColorAsString(const std::string & col)
	{
		_Color = convertColor (col.c_str());
	}

	// ***************************************************************************
	std::string CViewBitmap::getColorAsString() const
	{
		return NLMISC::toString(_Color.R) + " " + NLMISC::toString(_Color.G) + " " + NLMISC::toString(_Color.B) + " " + NLMISC::toString(_Color.A);
	}

	// ***************************************************************************
	void	CViewBitmap::setColorAsInt(sint32 col)
	{
		_Color.setPacked(col);
	}

	// ***************************************************************************
	sint32	CViewBitmap::getColorAsInt() const
	{
		return _Color.getPacked();
	}

	// ***************************************************************************
	void	CViewBitmap::setColorRGBA(NLMISC::CRGBA col)
	{
		_Color = col;
	}

	// ***************************************************************************
	NLMISC::CRGBA CViewBitmap::getColorRGBA() const
	{
		return _Color;
	}

	// ***************************************************************************
	sint32	CViewBitmap::getMaxUsedW() const
	{
		sint32 txw, txh;
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		rVR.getTextureSizeFromId (_TextureId, txw, txh);
		return txw;
	}

	// ***************************************************************************
	sint32	CViewBitmap::getMinUsedW() const
	{
		return getMaxUsedW();
	}

	// ***************************************************************************
	void CViewBitmap::serial(NLMISC::IStream &f)
	{
		CViewBase::serial(f);
		f.serial(_TextureId);
		f.serial(_Color);
		f.serial(_Rot);
		f.serialEnum(_Align);
		f.serialEnum(_Type);
		nlSerialBitBool(f, _Scale);
		nlSerialBitBool(f, _Flip);
		nlSerialBitBool(f, _Tile);
		nlSerialBitBool(f, _InheritGCAlpha);
		f.serial(_TxtOffsetX);
		f.serial(_TxtOffsetY);
		f.serial(_TxtWidth);
		f.serial(_TxtHeight);
	}

}

// ***************************************************************************
