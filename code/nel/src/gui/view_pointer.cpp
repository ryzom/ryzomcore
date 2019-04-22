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
#include "nel/gui/view_pointer.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/group_paragraph.h"
#include "nel/gui/group_container.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/misc/algo.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

NLMISC_REGISTER_OBJECT(CViewBase, CViewPointer, std::string, "generic_pointer");

namespace NLGUI
{

	bool CViewPointer::hwMouse = true;

	// --------------------------------------------------------------------------------------------------------------------
	CViewPointer::CViewPointer (const TCtorParam &param)
		: CViewPointerBase(param)
	{
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

		_OffsetX = 0;
		_OffsetY = 0;

		// The pointer must be draw over ALL layers
		_RenderLayer= VR_LAYER_MAX;
		_Color = CRGBA(255,255,255,255);
		_LastHightLight = NULL;
		_StringMode = false;
		_ForceStringMode = false;
		_StringCursor = NULL;
		_StringCursorHardware = NULL;
	}

	void CViewPointer::forceLink()
	{
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
		if (prop) _TxDefault = NLMISC::toLower ((const char *) prop);

 		prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_move_window");
		if (prop) _TxMoveWindow = NLMISC::toLower ((const char *) prop);

		prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_resize_BR_TL");
		if (prop) _TxResizeBRTL = NLMISC::toLower ((const char *) prop);

		prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_resize_BL_TR");
		if (prop) _TxResizeBLTR = NLMISC::toLower ((const char *) prop);

		prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_resize_TB");
		if (prop) _TxResizeTB = NLMISC::toLower ((const char *) prop);

		prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_resize_LR");
		if (prop) _TxResizeLR = NLMISC::toLower ((const char *) prop);

		prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_rotate");
		if (prop) _TxRotate = NLMISC::toLower ((const char *) prop);

		prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_scale");
		if (prop) _TxScale = NLMISC::toLower ((const char *) prop);

		prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_colpick");
		if (prop) _TxColPick = NLMISC::toLower ((const char *) prop);

		prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_pan");
		if (prop) _TxPan = NLMISC::toLower ((const char *) prop);

		prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_can_pan");
		if (prop) _TxCanPan = NLMISC::toLower ((const char *) prop);

		prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_pan_r2");
		if (prop) _TxPanR2 = NLMISC::toLower ((const char *) prop);

		prop = (char*) xmlGetProp (cur, (xmlChar*)"tx_can_pan_r2");
		if (prop) _TxCanPanR2 = NLMISC::toLower ((const char *) prop);

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

		CViewRenderer &rVR = *CViewRenderer::getInstance();

		if ( CWidgetManager::getInstance()->isIngame() )
		if (!_StringCursor)
		{
			// Create the string cursor instance
			std::vector<std::pair<std::string,std::string> > templateParams;
			templateParams.push_back (std::pair<std::string,std::string>("id", "string_cursor"));

			_StringCursor = CWidgetManager::getInstance()->getParser()->createGroupInstance("string_cursor", "", templateParams);
			if (_StringCursor)
				_StringCursor->setParentPos(CWidgetManager::getInstance()->getElementFromId("ui:interface"));

			templateParams.clear();
			templateParams.push_back (std::pair<std::string,std::string>("id", "string_cursor_hardware"));
			_StringCursorHardware = CWidgetManager::getInstance()->getParser()->createGroupInstance("string_cursor_hardware", "", templateParams);
			if (_StringCursorHardware)
				_StringCursorHardware->setParentPos(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
		}

		CRGBA col;
		if(getModulateGlobalColor())
			col.modulateFromColor (_Color, CWidgetManager::getInstance()->getGlobalColor());
		else
			col= _Color;

		//col.A = (uint8)(((sint32)col.A*((sint32)pIM->getGlobalColor().A+1))>>8);
		col.A = _Color.A;

		if (_LastHightLight != NULL)
		{
			_LastHightLight->setHighLighted(false,0);
			_LastHightLight = NULL;
		}

		if ( CWidgetManager::getInstance()->getCapturePointerLeft() != NULL && CWidgetManager::getInstance()->isMouseHandlingEnabled())
		{
			CCtrlMover *pCM = dynamic_cast<CCtrlMover*>( CWidgetManager::getInstance()->getCapturePointerLeft());
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
			_TxIdPanR2		= rVR.getTextureIdFromName (_TxPanR2);
			_TxIdCanPanR2	= rVR.getTextureIdFromName (_TxCanPanR2);

		}

		const vector<CCtrlBase *> &rICL = CWidgetManager::getInstance()->getCtrlsUnderPointer ();


		// Draw the captured cursor
		CCtrlBase *pCB = CWidgetManager::getInstance()->getCapturePointerLeft();
		if (pCB != NULL)
		{
			if (drawResizer(pCB,col)) return;
			if (drawColorPicker(pCB,col)) return;
			if (drawRotate(pCB,col)) return;
			if (drawPan(pCB,col)) return;
			if (drawCustom(pCB)) return;
			drawCursor(_TxIdDefault, col, 0);
			return;
		}

		const vector<CViewBase *> &vUP = CWidgetManager::getInstance()->getViewsUnderPointer ();

		for(uint i=0;i<vUP.size();i++)
		{
			CViewLink *vLink = dynamic_cast<CViewLink*>(vUP[i]);
			if (vLink != NULL)
			{
				string tooltip;
				uint8 rot;

				if (vLink->getMouseOverShape(tooltip, rot, col))
				{
					setString(ucstring::makeFromUtf8(tooltip));
					sint32 texId = rVR.getTextureIdFromName ("curs_pick.tga");

					CInterfaceGroup *stringCursor = hwMouse ? _StringCursorHardware : _StringCursor;
					if (stringCursor)
					{
						stringCursor->setX(_PointerX);
						stringCursor->setY(_PointerY);
						stringCursor->updateCoords();
						stringCursor->draw();
						// if in hardware mode, force to draw the default cursor no matter what..
						if ( hwMouse )
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
		pCB = CWidgetManager::getInstance()->getCapturePointerRight();
		if (pCB != NULL)
		{
			// Is it a 3d scene ?
			if (drawScale(pCB,col)) return;
			drawCursor(_TxIdDefault, col, 0);
			return;
		}

		bool overModalWindow = false;


		// is the cursor currently over a modal window ?
		CInterfaceGroup *currModal = CWidgetManager::getInstance()->getModalWindow();
		if (currModal)
		{
			sint32 xPos = _XReal + _OffsetX;
			sint32 yPos = _YReal + _OffsetY;
			overModalWindow = currModal->isIn(xPos, yPos, _WReal, _HReal);
		}

		// Draw the cursor type that are under the pointer
		if (CWidgetManager::getInstance()->isMouseHandlingEnabled())
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
							if (CWidgetManager::getInstance()->getCapturePointerLeft() != pCM)
								pGC->setHighLighted(true, 128);
							else
								pGC->setHighLighted(true, 255);
							_LastHightLight = pGC;
							break;
						}
					}
				}
			}
		}

		if (CWidgetManager::getInstance()->isMouseHandlingEnabled())
		{
			if (rICL.empty())
			{
				const vector<CInterfaceGroup *> &rIGL = CWidgetManager::getInstance()->getGroupsUnderPointer ();
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

		if (_StringMode && CWidgetManager::getInstance()->isMouseHandlingEnabled())
		{
			CInterfaceGroup *stringCursor = hwMouse ? _StringCursorHardware : _StringCursor;
			if (stringCursor)
			{
				stringCursor->setX(_PointerX);
				stringCursor->setY(_PointerY);
				stringCursor->updateCoords();
				stringCursor->draw();
				// if in hardware mode, force to draw the default cursor no matter what..
				if ( hwMouse )
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
				setString(ucstring::makeFromUtf8(tooltip));
				CViewRenderer &rVR = *CViewRenderer::getInstance();
				sint32 texId = rVR.getTextureIdFromName (texName);

				CInterfaceGroup *stringCursor = hwMouse ? _StringCursorHardware : _StringCursor;
				if (stringCursor)
				{
					stringCursor->setX(_PointerX);
					stringCursor->setY(_PointerY);
					stringCursor->updateCoords();
					stringCursor->draw();
					// if in hardware mode, force to draw the default cursor no matter what..
					if ( hwMouse )
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
				CViewRenderer &rVR = *CViewRenderer::getInstance();
				sint32 texId = rVR.getTextureIdFromName (texName);
				drawCursor(texId, col, 0);
				return true;
			}
		}
		return false;
	}


	// +++ SET +++


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
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		sint32 xPos = _XReal + _OffsetX;
		sint32 yPos = _YReal + _OffsetY;
		if ( !hwMouse )
		{
			rVR.draw11RotFlipBitmap (_RenderLayer, xPos, yPos, rot, false, texId, col);
		}
		else
		{
			// set new cursor for the hardware mouse
			std::string name = rVR.getTextureNameFromId(texId);
			rVR.getDriver()->setCursor(name, col, rot, (uint32) std::max(getX() - xPos, (sint32) 0), (uint32) std::max(getY() - yPos, (sint32) 0));
		}
	}


}

