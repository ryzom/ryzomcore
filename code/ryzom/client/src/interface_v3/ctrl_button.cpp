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

#include "ctrl_button.h"
#include "interface_manager.h"
#include "nel/misc/xml_auto_ptr.h"

// ----------------------------------------------------------------------------
using namespace std;
using namespace NLMISC;
using namespace NL3D;

NLMISC_REGISTER_OBJECT(CViewBase, CCtrlButton, std::string, "button");

// ----------------------------------------------------------------------------
bool CCtrlButton::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
{
	CXMLAutoPtr prop;
	//try to get props that can be inherited from groups
	//if a property is not defined, try to find it in the parent group.
	//if it is undefined, set it to zero
	if (! CCtrlBaseButton::parse(cur,parentGroup) )
	{
		string tmp = "cannot parse view:"+getId()+", parent:"+parentGroup->getId();
		nlinfo(tmp.c_str());
		return false;
	}


	// *** Read Textures
	prop = (char*) xmlGetProp( cur, (xmlChar*)"tx_normal" );
	if (prop)
	{
		string TxName = (const char *) prop;
		TxName = strlwr(TxName);
		_TextureIdNormal.setTexture(TxName.c_str());
	}

	prop = (char*) xmlGetProp( cur, (xmlChar*)"tx_pushed" );
	if (prop)
	{
		string TxName = (const char *) prop;
		TxName = strlwr(TxName);
		_TextureIdPushed.setTexture(TxName.c_str());
	}

	prop = (char*) xmlGetProp( cur, (xmlChar*)"tx_over" );
	if (prop)
	{
		string TxName = (const char *) prop;
		TxName = strlwr(TxName);
		_TextureIdOver.setTexture(TxName.c_str());
	}

	// *** Misc.
	prop = (char*) xmlGetProp( cur, (xmlChar*)"scale" );
	_Scale = false;
	if (prop)
		_Scale = convertBool(prop);


	prop = (char*) xmlGetProp (cur, (xmlChar*)"align");
	_Align = 0;
	if (prop)
	{
		const char *seekPtr = prop.getDatas();
		while (*seekPtr != 0)
		{
			if ((*seekPtr=='l')||(*seekPtr=='L'))
				_Align &= ~1;
			if ((*seekPtr=='r')||(*seekPtr=='R'))
				_Align |= 1;
			if ((*seekPtr=='b')||(*seekPtr=='B'))
				_Align &= ~2;
			if ((*seekPtr=='t')||(*seekPtr=='T'))
				_Align |= 2;
			++seekPtr;
		}
	}


	return true;
}

// ----------------------------------------------------------------------------
void CCtrlButton::draw ()
{
	sint32 nTxId = -1;
	CRGBA  color;

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();
	CRGBA  globalColor= pIM->getGlobalColorForContent();

	// *** Detect Over
	bool	lastOver = false;

	updateOver(lastOver);


	sint32 x = _XReal;
	sint32 y = _YReal;
	sint32 txw, txh;
	// the pointer is over the button
	if (_Scale)
	{
		x = _XReal;
		y = _YReal;
		txw = _WReal;
		txh = _HReal;
	}
	else
	{
		x = _XReal;
		y = _YReal;
	}

	switch(_Type)
	{
		case ToggleButton:
		{
			if (_Pushed)
			{
				nTxId = _TextureIdPushed;
				color  = getCurrentColorPushed(globalColor);
			}
			else
			{
				nTxId = _TextureIdNormal;
				color  = getCurrentColorNormal(globalColor);
			}
		}
		break;
		case RadioButton:
		{
//			CViewPointer &rIP = *CInterfaceManager::getInstance()->getPointer();
			// Init the radio button
			initRBRef();

			if (*_RBRef == this)
			{
				// if it is equal to the ref value, then the button must appear pushed
				nTxId = _TextureIdPushed;
				color  = getCurrentColorPushed(globalColor);
			}
			else
			{
				if ((_Over) && (pIM->getCapturePointerLeft() == this))
				{
					nTxId = _TextureIdPushed;
					color  = getCurrentColorPushed(globalColor);
				}
				else
				{
					nTxId = _TextureIdNormal;
					color     = getCurrentColorNormal(globalColor);
					_Pushed = false;
				}
			}
		}
		break;
		case PushButton:
		{
			if (_Over && (pIM->getCapturePointerLeft() == this))
			{
				nTxId = _TextureIdPushed;
				color  = getCurrentColorPushed(globalColor);
			}
			else
			{
				nTxId = _TextureIdNormal;
				color     = getCurrentColorNormal(globalColor);
				_Pushed = false;
			}
		}
		break;
		default:
		break;
	}

	color.A = (uint8)(((sint32)color.A*((sint32)globalColor.A+1))>>8);

	// Fromzen ?
	if (getFrozen() && getFrozenHalfTone())
		color.A >>= 2;

	if (!_Scale)
	{
		pIM->getViewRenderer().getTextureSizeFromId (nTxId, txw, txh);
		if (_Align&1)
			x = x + _WReal - txw;
		if (_Align&2)
			y = y + _HReal - txh;
	}
	rVR.drawRotFlipBitmap (	_RenderLayer, x, y, txw, txh,
							0, false,
							nTxId,
							color );

	if ((_OverWhenPushed == false) && (_Pushed == true || (pIM->getCapturePointerLeft() == this)))
		return;



	if (_Over)
	{

		if ((lastOver == false) && (_AHOnOver != NULL))
			pIM->runActionHandler (_AHOnOver, this, _AHOverParams);

		// the pointer is over the button
		color= getCurrentColorOver(globalColor);
		color.A = (uint8)(((sint32)color.A*((sint32)globalColor.A+1))>>8);

		// Frozen ?
		if (getFrozen())
			color.A >>= 2;

		// draw the over. force upper layer to avoid problem with DXTC/tga
		rVR.drawRotFlipBitmap (	_RenderLayer+1, x, y, txw, txh,
								0, false,
								_TextureIdOver,
								color );
	}
}


// ----------------------------------------------------------------------------
void CCtrlButton::updateCoords()
{
	if (!_Scale)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CViewRenderer &rVR = pIM->getViewRenderer();
		sint32 txw, txh;
		rVR.getTextureSizeFromId (_TextureIdNormal, txw, txh);
		_W = txw;
		_H = txh;
	}
	CViewBase::updateCoords();
}

// ----------------------------------------------------------------------------
void CCtrlButton::setTexture(const std::string&name)
{
//	CInterfaceManager *pIM = CInterfaceManager::getInstance();
//	CViewRenderer &rVR = pIM->getViewRenderer();
	_TextureIdNormal.setTexture(name.c_str());
}

// ----------------------------------------------------------------------------
void CCtrlButton::setTexturePushed(const std::string&name)
{
//	CInterfaceManager *pIM = CInterfaceManager::getInstance();
//	CViewRenderer &rVR = pIM->getViewRenderer();
	_TextureIdPushed.setTexture(name.c_str());
}

// ----------------------------------------------------------------------------
void CCtrlButton::setTextureOver(const std::string&name)
{
//	CInterfaceManager *pIM = CInterfaceManager::getInstance();
//	CViewRenderer &rVR = pIM->getViewRenderer();
	_TextureIdOver.setTexture(name.c_str());
}

// ----------------------------------------------------------------------------
std::string CCtrlButton::getTexture() const
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();
	return rVR.getTextureNameFromId(_TextureIdNormal);
}

// ----------------------------------------------------------------------------
std::string CCtrlButton::getTexturePushed() const
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();
	return rVR.getTextureNameFromId(_TextureIdPushed);
}

// ----------------------------------------------------------------------------
std::string CCtrlButton::getTextureOver() const
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();
	return rVR.getTextureNameFromId(_TextureIdOver);
}

// ***************************************************************************
sint32	CCtrlButton::getMaxUsedW() const
{
	sint32 txw, txh;
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();
	rVR.getTextureSizeFromId (_TextureIdNormal, txw, txh);
	return txw;
}

// ***************************************************************************
sint32	CCtrlButton::getMinUsedW() const
{
	return getMaxUsedW();
}

// ***************************************************************************
void CCtrlButton::fitTexture()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();
	sint32 w, h;
	rVR.getTextureSizeFromId(_TextureIdNormal, w, h);
	setW(w);
	setH(h);
}

// ***************************************************************************
bool CCtrlButton::getMouseOverShape(string &texName, uint8 &rot, CRGBA &col)
{
	if (_AHOnLeftClickString == "browse")
	{
		if (!_AHOnLeftClickStringParams.empty())
		{
			texName = "@curs_pick.tga@"+_AHOnLeftClickStringParams;
		}
		else
		{
			texName = "curs_pick.tga";
		}
		rot= 0;
		col = CRGBA::White;
		return true;
	}
	
	return false;
}
