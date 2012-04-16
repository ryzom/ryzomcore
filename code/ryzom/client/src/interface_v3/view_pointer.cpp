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

#include "../input.h"
//
#include "view_pointer.h"
#include "interface_manager.h"
#include "view_renderer.h"
#include "ctrl_col_pick.h"
#include "group_paragraph.h"
#include "group_html.h"
#include "group_map.h"
//
#include "nel/misc/xml_auto_ptr.h"
//
#include "group_container.h"
#include "interface_3d_scene.h"
//
#include "../r2/editor.h"




using namespace std;
using namespace NLMISC;

NLMISC_REGISTER_OBJECT(CViewBase, CViewPointer, std::string, "pointer");

// --------------------------------------------------------------------------------------------------------------------
CViewPointer::CViewPointer (const TCtorParam &param)
	: CViewBase(param),
	_Buttons(NLMISC::noButton)
{
	_PointerX = _PointerY = _PointerOldX = _PointerOldY = _PointerDownX = _PointerDownY = 0;
	_PointerDown = false;
	_PointerVisible = true;
	_TxIdDefault = -2;
	_TxIdMoveWindow = -2;
	_TxIdResizeBRTL = -2;
	_TxIdResizeBLTR = -2;
	_TxIdResizeTB = -2;
	_TxIdResizeLR = -2;
	_TxIdRotate = -2;
	_TxIdScale = -2;
	_TxIdColPick = -2;
	_TxIdPan = -2;
	_TxIdCanPan = -2;
	_TxIdPanR2 = -2;
	_TxIdCanPanR2 = -2;

	// The pointer must be draw over ALL layers
	_RenderLayer= VR_LAYER_MAX;
	_Color = CRGBA(255,255,255,255);
	_LastHightLight = NULL;
	_StringMode = false;
	_ForceStringMode = false;
	_StringCursor = NULL;
}


// +++ VIEW SPECIFIC +++

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointer::parse (xmlNodePtr cur,CInterfaceGroup * parentGroup)
{
	CXMLAutoPtr prop;

	if (! CViewBase::parse(cur, parentGroup) )
		return false;

	_OffsetX = getX();
	_OffsetY = getY();

	prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_default");
	if (prop) _TxDefault = (const char *) prop;
	_TxDefault = NLMISC::strlwr (_TxDefault);

 	prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_move_window");
	if (prop) _TxMoveWindow = (const char *) prop;
	_TxMoveWindow = NLMISC::strlwr (_TxMoveWindow);

	prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_resize_BR_TL");
	if (prop) _TxResizeBRTL = (const char *) prop;
	_TxResizeBRTL = NLMISC::strlwr (_TxResizeBRTL);

	prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_resize_BL_TR");
	if (prop) _TxResizeBLTR = (const char *) prop;
	_TxResizeBLTR = NLMISC::strlwr (_TxResizeBLTR);

	prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_resize_TB");
	if (prop) _TxResizeTB = (const char *) prop;
	_TxResizeTB = NLMISC::strlwr (_TxResizeTB);

	prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_resize_LR");
	if (prop) _TxResizeLR = (const char *) prop;
	_TxResizeLR = NLMISC::strlwr (_TxResizeLR);

	prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_rotate");
	if (prop) _TxRotate = (const char *) prop;
	_TxRotate = NLMISC::strlwr (_TxRotate);

	prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_scale");
	if (prop) _TxScale = (const char *) prop;
	_TxScale = NLMISC::strlwr (_TxScale);

	prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_colpick");
	if (prop) _TxColPick = (const char *) prop;
	_TxColPick = NLMISC::strlwr (_TxColPick);

	prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_pan");
	if (prop) _TxPan = (const char *) prop;
	_TxPan = NLMISC::strlwr (_TxPan);

	prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_can_pan");
	if (prop) _TxCanPan = (const char *) prop;
	_TxCanPan = NLMISC::strlwr (_TxCanPan);

	if (ClientCfg.R2EDEnabled)
	{
		prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_pan_r2");
		if (prop) _TxPanR2 = (const char *) prop;
		_TxPanR2 = NLMISC::strlwr (_TxPanR2);

		prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_can_pan_r2");
		if (prop) _TxCanPanR2 = (const char *) prop;
		_TxCanPanR2 = NLMISC::strlwr (_TxCanPanR2);
	}

	prop = (char*) xmlGetProp (cur, (xmlChar*)"color");
	if (prop) _Color = convertColor(prop);

	return true;
}

// --------------------------------------------------------------------------------------------------------------------
class CCtrlDepthEntry
{
public:
	CCtrlBase		*Ctrl;
	uint			Depth;
	bool	operator<(const CCtrlDepthEntry &o) const
	{
		// Inverse Test => descending order
		return Depth>o.Depth;
	}
};

// --------------------------------------------------------------------------------------------------------------------
void CViewPointer::draw ()
{
	// Do not display the pointer if not visible.
	if(!_PointerVisible)
		return;

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();

	if (pIM->isInGame())
	if (!_StringCursor)
	{
		// Create the string cursor instance
		std::vector<std::pair<std::string,std::string> > templateParams;
		templateParams.push_back (std::pair<std::string,std::string>("id", "string_cursor"));

		_StringCursor = pIM->createGroupInstance("string_cursor", "", templateParams);
		if (_StringCursor)
			_StringCursor->setParentPos(pIM->getElementFromId("ui:interface"));

		templateParams.clear();
		templateParams.push_back (std::pair<std::string,std::string>("id", "string_cursor_hardware"));
		_StringCursorHardware = pIM->createGroupInstance("string_cursor_hardware", "", templateParams);
		if (_StringCursorHardware)
			_StringCursorHardware->setParentPos(pIM->getElementFromId("ui:interface"));
	}

	CRGBA col;
	if(getModulateGlobalColor())
		col.modulateFromColor (_Color, pIM->getGlobalColor());
	else
		col= _Color;

	//col.A = (uint8)(((sint32)col.A*((sint32)pIM->getGlobalColor().A+1))>>8);
	col.A = _Color.A;

	if (_LastHightLight != NULL)
	{
		_LastHightLight->setHighLighted(false,0);
		_LastHightLight = NULL;
	}

	if (pIM->getCapturePointerLeft() != NULL && pIM->isMouseHandlingEnabled())
	{
		CCtrlMover *pCM = dynamic_cast<CCtrlMover*>(pIM->getCapturePointerLeft());
		if ((pCM != NULL) && (pCM->canMove() == true))
		{
			CGroupContainer *pGC = dynamic_cast<CGroupContainer *>(pCM->getParent());
			if (pGC != NULL && !pGC->isLocked())
			{
				pGC->setHighLighted(true, 255);
				_LastHightLight = pGC;
			}
		}
	}

	if (_TxIdDefault == -2)
	{
		_TxIdDefault	= rVR.getTextureIdFromName (_TxDefault);
		_TxIdMoveWindow = rVR.getTextureIdFromName (_TxMoveWindow);
		_TxIdResizeBRTL = rVR.getTextureIdFromName (_TxResizeBRTL);
		_TxIdResizeBLTR = rVR.getTextureIdFromName (_TxResizeBLTR);
		_TxIdResizeTB	= rVR.getTextureIdFromName (_TxResizeTB);
		_TxIdResizeLR	= rVR.getTextureIdFromName (_TxResizeLR);
		_TxIdRotate		= rVR.getTextureIdFromName (_TxRotate);
		_TxIdScale		= rVR.getTextureIdFromName (_TxScale);
		_TxIdColPick	= rVR.getTextureIdFromName (_TxColPick);
		_TxIdPan		= rVR.getTextureIdFromName (_TxPan);
		_TxIdCanPan		= rVR.getTextureIdFromName (_TxCanPan);
		if (ClientCfg.R2EDEnabled)
		{
			_TxIdPanR2		= rVR.getTextureIdFromName (_TxPanR2);
			_TxIdCanPanR2	= rVR.getTextureIdFromName (_TxCanPanR2);
		}
	}

	const vector<CCtrlBase *> &rICL = pIM->getCtrlsUnderPointer ();


	// Draw the captured cursor
	CCtrlBase *pCB = pIM->getCapturePointerLeft();
	if (pCB != NULL)
	{
		if (drawResizer(pCB,col)) return;
		//if (drawMover(pCB,col)) return;
		if (drawColorPicker(pCB,col)) return;
		if (drawRotate(pCB,col)) return;
		if (drawPan(pCB,col)) return;
		if (drawCustom(pCB)) return;
		drawCursor(_TxIdDefault, col, 0);
		return;
	}

	const vector<CViewBase *> &vUP = pIM->getViewsUnderPointer ();

	for(uint i=0;i<vUP.size();i++)
	{
		CViewLink *vLink = dynamic_cast<CViewLink*>(vUP[i]);
		if (vLink != NULL)
		{
			string tooltip;
			uint8 rot;

			if (vLink->getMouseOverShape(tooltip, rot, col))
			{
				setString(ucstring(tooltip));
				sint32 texId = rVR.getTextureIdFromName ("curs_pick.tga");

				CInterfaceGroup *stringCursor = IsMouseCursorHardware() ? _StringCursorHardware : _StringCursor;
				if (stringCursor)
				{
					stringCursor->setX(_PointerX);
					stringCursor->setY(_PointerY);
					stringCursor->updateCoords();
					stringCursor->draw();
					// if in hardware mode, force to draw the default cursor no matter what..
					if (IsMouseCursorHardware())
						drawCursor(texId, col, 0);
				}
				else
				{
					drawCursor(texId, col, 0);
				}
				return;
			}
		}
	}

	// Draw if capture right
	pCB = pIM->getCapturePointerRight();
	if (pCB != NULL)
	{
		// Is it a 3d scene ?
		if (drawScale(pCB,col)) return;
		drawCursor(_TxIdDefault, col, 0);
		return;
	}

	bool overModalWindow = false;


	// is the cursor currently over a modal window ?
	CInterfaceGroup *currModal = pIM->getModalWindow();
	if (currModal)
	{
		sint32 xPos = _XReal + _OffsetX;
		sint32 yPos = _YReal + _OffsetY;
		overModalWindow = currModal->isIn(xPos, yPos, _WReal, _HReal);
	}

	// Draw the cursor type that are under the pointer
	if (pIM->isMouseHandlingEnabled())
	{
		// Sorts the controls according to their depth, to approximate as best the CapturePointerLeft algo.
		// Especially important so that Resizers controls get the precedence over the move control (else could randomly bug like in chat group)
		static vector<CCtrlDepthEntry>		sortedControls;
		sortedControls.clear();
		for(uint i=0;i<rICL.size();i++)
		{
			CCtrlDepthEntry		cde;
			cde.Ctrl= rICL[i];
			// NB: not the exact CInterfaceManager getDepth test here, but should work fine
			cde.Depth= cde.Ctrl->getParentDepth() + cde.Ctrl->getDeltaDepth();
			sortedControls.push_back(cde);
		}
		std::sort(sortedControls.begin(), sortedControls.end());

		// Then draw the correct cursor
		for (uint32 i = 0; i < sortedControls.size(); ++i)
		{
			CCtrlBase *pCB = sortedControls[i].Ctrl;

			if (overModalWindow)
			{
				if (!pCB->isSonOf(currModal)) continue;
			}

			if (drawBrowse(pCB, col)) return;
			if (drawResizer(pCB,col)) return;
			if (drawColorPicker(pCB,col)) return;
			if (drawLink (pCB, col)) return;
			if (drawCustom(pCB)) return;

			// test for move highlight
			if (_LastHightLight == NULL)
			{
				CCtrlMover *pCM = dynamic_cast<CCtrlMover*>(pCB);
				if ( (pCM != NULL) && (pCM->canMove() == true) )
				{
					CGroupContainer *pGC = dynamic_cast<CGroupContainer *>(pCM->getParent());
					if (pGC != NULL && !pGC->isLocked())
					{
						if (pIM->getCapturePointerLeft() != pCM)
							pGC->setHighLighted(true, 128);
						else
							pGC->setHighLighted(true, 255);
						_LastHightLight = pGC;
						break;
					}
				}
			}

			//if (drawMover(pCB,col)) return;
		}
	}

	if (pIM->isMouseHandlingEnabled())
	{
		if (rICL.empty())
		{
			const vector<CInterfaceGroup *> &rIGL = pIM->getGroupsUnderPointer ();
			for (uint32 i = 0; i < rIGL.size(); ++i)
			{
				CInterfaceGroup *pG = rIGL[i];
				if (overModalWindow)
				{
					if (!pG->isSonOf(currModal)) continue;
				}
				if (drawPan (pG, col)) return;
				if (drawBrowse(pG, col)) return;
			}
		}
	}

	if (_StringMode && pIM->isMouseHandlingEnabled())
	{
		CInterfaceGroup *stringCursor = IsMouseCursorHardware() ? _StringCursorHardware : _StringCursor;
		if (stringCursor)
		{
			stringCursor->setX(_PointerX);
			stringCursor->setY(_PointerY);
			stringCursor->updateCoords();
			stringCursor->draw();
			// if in hardware mode, force to draw the default cursor no matter what..
			if (IsMouseCursorHardware())
			{
				drawCursor(_TxIdDefault, col, 0);
			}
		}
	}
	else
	{
		// Draw the default cursor
		drawCursor(_TxIdDefault, col, 0);
	}
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointer::drawResizer(CCtrlBase* pCB, CRGBA col)
{
	CCtrlResizer *pCR = dynamic_cast<CCtrlResizer*>(pCB);
	if (pCR != NULL)
	{
		CGroupContainer *parent = dynamic_cast<CGroupContainer *>(pCR->getParent());
		if (parent && !parent->isLocked())
		{
			sint32 texID= -1;
			switch(pCR->getRealResizerPos())
			{
				case Hotspot_BR:
				case Hotspot_TL:
					texID = _TxIdResizeBRTL;
				break;
				case Hotspot_BL:
				case Hotspot_TR:
					texID = _TxIdResizeBLTR;
				break;
				case Hotspot_MR:
				case Hotspot_ML:
					texID = _TxIdResizeLR;
				break;
				case Hotspot_TM:
				case Hotspot_BM:
					texID = _TxIdResizeTB;
				break;
				default:
					return false;
				break;
			}
			drawCursor(texID, col, false);
			return true;
		}
	}
	return false;
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointer::drawMover(CCtrlBase* pCB, CRGBA col)
{
	CCtrlMover *pCM = dynamic_cast<CCtrlMover*>(pCB);
	if ((pCM != NULL) && (pCM->canMove() == true))
	{
		drawCursor(_TxIdMoveWindow, col, 0);
		return true;
	}
	return false;
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointer::drawRotate (CCtrlBase* pCB, CRGBA col)
{
	CInterface3DScene *pI3DS = dynamic_cast<CInterface3DScene *>(pCB);
	if (pI3DS != NULL)
	{
		drawCursor(_TxIdRotate, col, 0);
		return true;
	}
	return false;
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointer::drawScale (CCtrlBase* pCB, CRGBA col)
{
	CInterface3DScene *pI3DS = dynamic_cast<CInterface3DScene *>(pCB);
	if (pI3DS != NULL)
	{
		drawCursor(_TxIdScale, col, 0);
		return true;
	}
	return false;
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointer::drawColorPicker (CCtrlBase* pCB, CRGBA col)
{
	CCtrlColPick *pCCP = dynamic_cast<CCtrlColPick*>(pCB);
	if (pCCP != NULL)
	{
		drawCursor(_TxIdColPick, col, 0);
		return true;
	}
	return false;
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointer::drawLink (CCtrlBase* pCB, CRGBA col)
{
	CCtrlLink *pCCP = dynamic_cast<CCtrlLink*>(pCB);
	if (pCCP != NULL)
	{
		drawCursor(_TxIdColPick, col, 0);
		return true;
	}
	return false;
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointer::drawBrowse (CCtrlBase* pCB, CRGBA col)
{
	CGroupHTML *pCGH = dynamic_cast<CGroupHTML *>(pCB);
	if (pCGH != NULL)
	{
		if (pCGH->isBrowsing())
		{
			static uint8 rot =0;
			drawCursor(_TxIdRotate, col, rot>>3);
			rot = (rot+1) & 0x1f;
			return true;
		}
	}
	return false;
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointer::drawPan(CCtrlBase* pCB, NLMISC::CRGBA col)
{
	CGroupMap *gm = dynamic_cast<CGroupMap *>(pCB);
	if (gm)
	{
		sint32 texId;
		if (ClientCfg.R2EDEnabled && R2::getEditor().getCurrentTool())
		{
			/** If cursor is set to anything other than the default cursor, use that cursor (because action can be performed on the map
			  * by the current tool
			  */
			if (_TxDefault == "curs_default.tga")
			{
				texId = gm->isPanning() ? _TxIdPanR2 : _TxIdCanPanR2;
			}
			else return false;
		}
		else
		{
			texId = gm->isPanning() ? _TxIdPan : _TxIdCanPan;
		}
		drawCursor(texId, col, 0);
		return true;
	}
	return false;
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointer::drawCustom(CCtrlBase* pCB)
{
	string texName;
	uint8 rot;
	NLMISC::CRGBA col;
	if (pCB->getMouseOverShape(texName, rot, col))
	{
		if (texName[0] == '@')
		{
			const string &tooltipInfos = texName.substr(1);
			string tooltip;
			vector<string> tooltipInfosList;
			splitString(tooltipInfos, "@", tooltipInfosList);
			texName = tooltipInfosList[0];
			tooltip = tooltipInfosList[1];
			nlinfo(tooltip.c_str());
			setString(ucstring(tooltip));
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			CViewRenderer &rVR = pIM->getViewRenderer();
			sint32 texId = rVR.getTextureIdFromName (texName);

			CInterfaceGroup *stringCursor = IsMouseCursorHardware() ? _StringCursorHardware : _StringCursor;
			if (stringCursor)
			{
				stringCursor->setX(_PointerX);
				stringCursor->setY(_PointerY);
				stringCursor->updateCoords();
				stringCursor->draw();
				// if in hardware mode, force to draw the default cursor no matter what..
				if (IsMouseCursorHardware())
					drawCursor(texId, col, 0);
			}
			else
			{
				drawCursor(texId, col, 0);
			}
			return true;
		}
		else
		{
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			CViewRenderer &rVR = pIM->getViewRenderer();
			sint32 texId = rVR.getTextureIdFromName (texName);
			drawCursor(texId, col, 0);
			return true;
		}
	}
	return false;
}


// +++ SET +++

// --------------------------------------------------------------------------------------------------------------------
void CViewPointer::setPointerPos (sint32 x, sint32 y)
{
	if (_PointerDown)
	{
		if (!_PointerDrag)
		{
			if (((_PointerX - _PointerDownX) != 0) ||
				((_PointerY - _PointerDownY) != 0))
			{
				_PointerDrag = true;
			}
		}
	}

	_PointerOldX = getX();
	_PointerOldY = getY();

	_PointerX = x;
	_PointerY = y;
}

// --------------------------------------------------------------------------------------------------------------------
void CViewPointer::setPointerDispPos (sint32 x, sint32 y)
{
	setX (x);
	setY (y);
	updateCoords ();
}

// --------------------------------------------------------------------------------------------------------------------
void CViewPointer::resetPointerPos ()
{
	_PointerOldX = _PointerX;
	_PointerOldY = _PointerY;
}

// --------------------------------------------------------------------------------------------------------------------
void CViewPointer::setPointerDown (bool pd)
{
	_PointerDown = pd;

	if (_PointerDown == true)
	{
		_PointerDownX = _PointerX;
		_PointerDownY = _PointerY;
	}

	if (_PointerDown == false)
		_PointerDrag = false;
}

// --------------------------------------------------------------------------------------------------------------------
void CViewPointer::setPointerDownString (const std::string &s)
{
	_PointerDownString = s;
}

// +++ GET +++

// --------------------------------------------------------------------------------------------------------------------
void CViewPointer::getPointerPos (sint32 &x, sint32 &y)
{
	x = _PointerX;
	y = _PointerY;
}

// --------------------------------------------------------------------------------------------------------------------
void CViewPointer::getPointerDispPos (sint32 &x, sint32 &y)
{
	x = getX();
	y = getY();
}

// --------------------------------------------------------------------------------------------------------------------
void CViewPointer::getPointerOldPos (sint32 &x, sint32 &y)
{
	x = _PointerOldX;
	y = _PointerOldY;
}

// --------------------------------------------------------------------------------------------------------------------
void CViewPointer::getPointerDownPos (sint32 &x, sint32 &y)
{
	x = _PointerDownX;
	y = _PointerDownY;
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointer::getPointerDown ()
{
	return _PointerDown;
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointer::getPointerDrag ()
{
	return _PointerDrag;
}

// --------------------------------------------------------------------------------------------------------------------
std::string CViewPointer::getPointerDownString ()
{
	return _PointerDownString;
}

// --------------------------------------------------------------------------------------------------------------------
void CViewPointer::setStringMode (bool stringCursor)
{
	_StringMode = stringCursor;
}

// --------------------------------------------------------------------------------------------------------------------
void CViewPointer::setString (const ucstring &str, CInterfaceGroup *target)
{
	if (target)
	{
		CInterfaceElement *element = target->getView ("fake_txt");
		if (element)
		{
			CViewText *text = dynamic_cast<CViewText*> (element);
			if (text)
				text->setText(str);
		}
		element = target->getView ("real_txt");
		if (element)
		{
			CViewText *text = dynamic_cast<CViewText*> (element);
			if (text)
				text->setText(str);
		}
		target->updateCoords();
		target->updateCoords();
		_ContextString = str;
	}
}


// --------------------------------------------------------------------------------------------------------------------
void CViewPointer::setString (const ucstring &str)
{
	if (_ContextString != str)
	{
		setString(str, _StringCursor);
		setString(str, _StringCursorHardware);
	}
}

// --------------------------------------------------------------------------------------------------------------------
void CViewPointer::drawCursor(sint32 texId, NLMISC::CRGBA col, uint8 rot)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();
	sint32 xPos = _XReal + _OffsetX;
	sint32 yPos = _YReal + _OffsetY;
	if (!IsMouseCursorHardware())
	{
		rVR.draw11RotFlipBitmap (_RenderLayer, xPos, yPos, rot, false, texId, col);
	}
	else
	{
		// set new cursor for the hardware mouse
		std::string name = rVR.getTextureNameFromId(texId);
		Driver->setCursor(name, col, rot, (uint32) std::max(getX() - xPos, (sint32) 0), (uint32) std::max(getY() - yPos, (sint32) 0));
	}
}


