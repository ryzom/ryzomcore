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
#include "interface_manager.h"
#include "ctrl_col_pick.h"
#include "game_share/xml_auto_ptr.h"

using namespace NLMISC;
using namespace std;

NLMISC_REGISTER_OBJECT(CViewBase, CCtrlColPick, std::string, "colpick");

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
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CViewRenderer &rVR = pIM->getViewRenderer();
		rVR.deleteTexture (_Texture);
	}
}

// ------------------------------------------------------------------------------------------------
bool CCtrlColPick::parse(xmlNodePtr node, CInterfaceGroup * parentGroup)
{
	if (!CCtrlBase::parse(node, parentGroup))
		return false;

	CXMLAutoPtr prop;
	// Read textures
	prop = (char*) xmlGetProp( node, (xmlChar*)"texture" );
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();
	if(prop)
	{
		string sTmp = NLMISC::strlwr((const char*)prop);
		_Texture = rVR.createTexture (sTmp, 0, 0, 256, 64, false, false);
	}

	prop = (char*) xmlGetProp( node, (xmlChar*)"onchange" );
	if (prop)	_AHOnChange = NLMISC::strlwr(prop);
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
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();
	sint32 txw, txh;
	rVR.getTextureSizeFromId (_Texture, txw, txh);
	_W = txw;
	_H = txh;
	CCtrlBase::updateCoords();
}

// ------------------------------------------------------------------------------------------------
void CCtrlColPick::draw()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();

	CRGBA col = CRGBA(255,255,255,(uint8)pIM->getGlobalColor().A);

	rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal,
							_WReal, _HReal,
							0, false,
							_Texture,
							col );
}

// ------------------------------------------------------------------------------------------------
bool CCtrlColPick::handleEvent (const CEventDescriptor &event)
{
	if (CCtrlBase::handleEvent(event)) return true;
	if (!_Active)
		return false;
	if (event.getType() == CEventDescriptor::mouse)
	{
		const CEventDescriptorMouse &eventDesc = (const CEventDescriptorMouse &)event;
		if ((CInterfaceManager::getInstance()->getCapturePointerLeft() != this) &&
			(!((eventDesc.getX() >= _XReal) &&
			(eventDesc.getX() < (_XReal + _WReal))&&
			(eventDesc.getY() > _YReal) &&
			(eventDesc.getY() <= (_YReal+ _HReal)))))
			return false;

		if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftdown)
		{
			_MouseDown = true;
			selectColor(eventDesc.getX()-_XReal, eventDesc.getY()-_YReal);
			return true;
		}
		if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftup)
		{
			_MouseDown = false;
			return true;
		}
		if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mousemove)
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
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

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
		pIM->runActionHandler(_AHOnChange, this, _AHOnChangeParams);
}

// ------------------------------------------------------------------------------------------------
CRGBA CCtrlColPick::getColor (sint32 x, sint32 y)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();

	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x >= _WReal) x = _WReal-1;
	if (y >= _HReal) y = _HReal-1;
	return rVR.getTextureColor (_Texture, x, y);
}
