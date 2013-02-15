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



// ----------------------------------------------------------------------------

#include "stdpch.h"

#include "view_bitmap.h"
#include "interface_manager.h"
#include "game_share/xml_auto_ptr.h"
#include "group_container.h"

// ----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NL3D;


NLMISC_REGISTER_OBJECT(CViewBase, CViewBitmap, std::string, "bitmap");
REGISTER_UI_CLASS(CViewBitmap)

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
		//CViewRenderer &rVR = pIM->getViewRenderer();
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
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();

	CRGBA col;
	if(getModulateGlobalColor())
	{
		col.modulateFromColor (_Color, pIM->getGlobalColorForContent());
	}
	else
	{
		col= _Color;
		col.A = (uint8)(((sint32)col.A*((sint32)pIM->getGlobalColorForContent().A+1))>>8);
	}

	if (_InheritGCAlpha)
	{
		// search a parent container
		CInterfaceGroup *gr = getParent();
		while (gr)
		{
			if (gr->isGroupContainer())
			{
				CGroupContainer *gc = static_cast<CGroupContainer *>(gr);
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
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CViewRenderer &rVR = pIM->getViewRenderer();
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
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();
	return rVR.getTextureNameFromId (_TextureId);
}

// ***************************************************************************
void CViewBitmap::fitTexture()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();
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
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();
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


// ***************************************************************************
