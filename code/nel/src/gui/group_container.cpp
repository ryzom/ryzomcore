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
#include "nel/gui/group_container.h"
#include "nel/gui/interface_options.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/view_text_formated.h"
#include "nel/gui/view_text_id.h"
#include "nel/gui/lua_ihm.h"
#include "nel/gui/group_list.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/ctrl_scroll.h"
#include "nel/gui/view_text.h"
#include "nel/gui/view_bitmap.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/view_pointer_base.h"
#include "nel/misc/i18n.h"
#include "nel/misc/algo.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;

namespace
{
	const sint SIZE_W_LEFT = 16;
	const sint DELTA_BEFORE_POPUP = 32;
	const sint DELTA_BEFORE_MOVING_IN_PARENT_LIST = 16;
}

namespace NLGUI
{

	bool CGroupContainer::_ValidateCanDeactivate = true;

	//#define DRAW_GC_TEST_QUADS


	#ifdef DRAW_GC_TEST_QUADS
		static void drawGCTestQuad(sint renderLayer, sint32 xreal, sint32 yreal, sint32 wreal, sint32 hreal, CRGBA color)
		{
			CViewRenderer &rVR = *CViewRenderer::getInstance();
			if(rVR.isMinimized())
				return;
			sint32 x, y, w, h;
			rVR.getClipWindow(x, y, w, h);
			uint32 sw, sh;
			rVR.getScreenSize(sw, sh);
			rVR.setClipWindow(0, 0, (sint32) sw, (sint32) sh);
			rVR.drawRotFlipBitmap (renderLayer, xreal, yreal, wreal, hreal, 0, false, rVR.getBlankTextureId(), color );
			rVR.setClipWindow(x, y, w ,h);
		}
	#endif


	// ***************************************************************************
	const string		CGroupContainer::_OptionLayerName[CGroupContainer::NumLayerName]=
	{
		"layer0",
		"layer1",
		"layer2",
		"layer3",
		"layer4",
		"layer5",
		"layer6",
		"layer7",
		"layer8",
		"layer9",
	};


	// ***************************************************************************
	// CCtrlResizer
	// ***************************************************************************

	// ***************************************************************************
	CCtrlResizer::CCtrlResizer(const TCtorParam &param)
	: CCtrlBase(param)
	{
		WMin = WMax = 0;
		HMin = HMax = 0;
		_ResizerPos = Hotspot_BR;
		IsMaxH = false;
		_MouseDown = false;
		_XBias = 0;
		_YBias = 0;
		resizer = true;
	}


	// ***************************************************************************
	THotSpot CCtrlResizer::getRealResizerPos() const
	{
		CGroupContainer *parent = dynamic_cast<CGroupContainer *>(getParent());
		if (parent)
		{
			THotSpot resizerPos = _ResizerPos;
			if (!IsMaxH && parent->getPopupMinH() == parent->getPopupMaxH())
			{
				resizerPos = (THotSpot) (resizerPos & ~(Hotspot_Bx | Hotspot_Mx | Hotspot_Tx));
			}
			if (parent->getPopupMinW() == parent->getPopupMaxW())
			{
				resizerPos = (THotSpot) (resizerPos & ~(Hotspot_xR | Hotspot_xM | Hotspot_xL));
			}
			return resizerPos;
		}
		return _ResizerPos;
	}

	// ***************************************************************************
	void CCtrlResizer::draw ()
	{
		#ifdef DRAW_GC_TEST_QUADS
			CRGBA col;
			switch(ResizerPos)
			{
				case Hotspot_TR: col = CRGBA::Yellow; break;
				case Hotspot_MR: col = CRGBA::Blue; break;
				case Hotspot_BR: col = CRGBA::Yellow; break;
				case Hotspot_BM: col = CRGBA::Blue; break;
				case Hotspot_BL: col = CRGBA::Yellow; break;
				case Hotspot_ML: col = CRGBA::Blue; break;
				case Hotspot_TL: col = CRGBA::Yellow; break;
				case Hotspot_TM: col = CRGBA::Blue; break;
			}

			drawGCTestQuad(_RenderLayer, _XReal, _YReal, _WReal, _HReal, col);
		#endif
	}

	// ***************************************************************************
	bool CCtrlResizer::handleEvent (const NLGUI::CEventDescriptor &event)
	{
		if (CCtrlBase::handleEvent(event)) return true;
		if (!_Active || !_Parent)
			return false;

		if (event.getType() == NLGUI::CEventDescriptor::system)
		{
			const NLGUI::CEventDescriptorSystem &eds = (const NLGUI::CEventDescriptorSystem &) event;
			if (eds.getEventTypeExtended() == NLGUI::CEventDescriptorSystem::setfocus)
			{
				const NLGUI::CEventDescriptorSetFocus &edsf = (const NLGUI::CEventDescriptorSetFocus &) eds;
				if (edsf.hasFocus() == false && _MouseDown)
				{
					_MouseDown = false;
					_Parent->invalidateCoords();
					return true;
				}
			}
		}

		if (event.getType() == NLGUI::CEventDescriptor::mouse)
		{
			const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;
			if ((CWidgetManager::getInstance()->getCapturePointerLeft() != this) && !isIn(eventDesc.getX(), eventDesc.getY()))
				return false;

			CGroupContainer *gc = dynamic_cast<CGroupContainer *>(_Parent);

			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftdown)
			{
				// must check that parent isn't closed
				if (gc)
				{

					if (!gc->isOpen()) return false;
					if (gc->getLayerSetup() != 0) return false;
					if (gc->isLocked()) return true;
					if (IsMaxH)
						gc->setPopupMaxH(gc->getH());
				}
				_MouseDown = true;
				_MouseDownX = eventDesc.getX();
				_MouseDownY = eventDesc.getY();
				_XBias = 0;
				_YBias = 0;
				return true;
			}

			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup)
			{
				_MouseDown = false;
				_Parent->invalidateCoords();
				return true;
			}
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousemove)
			{
				if (_MouseDown)
				{
					//nlinfo("x = %d, y = %d", eventDesc.getX() + _XReal, eventDesc.getY() + _YReal);
					sint32 dx = eventDesc.getX() - _MouseDownX;
					sint32 dy = eventDesc.getY() - _MouseDownY;


					THotSpot resizerPos = getRealResizerPos();

					// horizontal resize
					if (dx != 0)
					{
						if (_XBias > 0)
						{
							_XBias += dx;
							if (_XBias < 0)
							{
								dx = _XBias;
								_XBias = 0;
							}
							else
							{
								 dx = 0;
							}
						}
						else if (_XBias < 0)
						{
							_XBias += dx;
							if (_XBias > 0)
							{
								dx = _XBias;
								_XBias = 0;
							}
							else
							{
								 dx = 0;
							}
						}
						if (resizerPos & Hotspot_xR)
						{
							sint32 effectiveDX = resizeW (dx);
							if (effectiveDX != 0 && gc) gc->touch();
							if (_Parent->getPosRef() & Hotspot_xR)
							{
								_Parent->setX(_Parent->getX() + effectiveDX);
							}
							_XBias += dx - effectiveDX;
						}
						else if (resizerPos & Hotspot_xL)
						{
							sint32 effectiveDX = resizeW (- dx);
							if (effectiveDX != 0 && gc) gc->touch();
							if (_Parent->getPosRef() & Hotspot_xL)
							{
								_Parent->setX(_Parent->getX() - effectiveDX);
							}
							_XBias += dx + effectiveDX;
						}
					}

					// vertical resize
					if (dy != 0)
					{
						if (_YBias > 0)
						{
							_YBias += dy;
							if (_YBias < 0)
							{
								dy = _YBias;
								_YBias = 0;
							}
							else
							{
								 dy = 0;
							}
						}
						else if (_YBias < 0)
						{
							_YBias += dy;
							if (_YBias > 0)
							{
								dy = _YBias;
								_YBias = 0;
							}
							else
							{
								 dy = 0;
							}
						}
						if (resizerPos & Hotspot_Tx)
						{
							sint32 effectiveDY = resizeH (dy);
							if (effectiveDY != 0 && gc) gc->touch();
							if (_Parent->getPosRef() & Hotspot_Tx)
							{
								_Parent->setY(_Parent->getY() + effectiveDY);
							}
							_YBias += dy - effectiveDY;
						}
						else if (resizerPos & Hotspot_Bx)
						{
							sint32 effectiveDY = resizeH (- dy);
							if (effectiveDY != 0 && gc) gc->touch();
							if (_Parent->getPosRef() & Hotspot_Bx)
							{
								_Parent->setY(_Parent->getY() + effectiveDY);
							}
							_YBias += dy + effectiveDY;
						}
					}

					_Parent->invalidateCoords();

					// update pos
					_MouseDownX = eventDesc.getX();
					_MouseDownY = eventDesc.getY();

					//
					// call resize handler of parent container if any
					if (gc && gc->getAHOnResizePtr() != NULL)
					{
						CAHManager::getInstance()->runActionHandler(gc->getAHOnResize(), gc, gc->getAHOnResizeParams());
					}
				}
				return true;
			}
		}
		return false;
	}

	// ***************************************************************************
	sint32 CCtrlResizer::resizeW (sint32 dx)
	{
		sint32 newW = _Parent->getW();
		newW += dx;
		sint32 clippedNewW = newW;
		NLMISC::clamp(clippedNewW, WMin, WMax);
		// clip by screen
		uint32 sw, sh;
		CViewRenderer &vr = *CViewRenderer::getInstance();
		vr.getScreenSize(sw, sh);
		if (_Parent->getPosRef() & Hotspot_xR)
		{
			if (_ResizerPos & Hotspot_xR)
			{
				clippedNewW = std::min((sint32) sw + _Parent->getW() - _Parent->getXReal(), clippedNewW);
			}
			else
			{
				clippedNewW = std::min(clippedNewW, _Parent->getXReal());
			}
		}
		else
		{
			if (_ResizerPos & Hotspot_xL)
			{
				clippedNewW = std::min(clippedNewW, _Parent->getXReal() +  _Parent->getW());
			}
			else
			{
				clippedNewW = std::min((sint32) sw - _Parent->getXReal(), clippedNewW);
			}
		}
		//
		dx = clippedNewW - _Parent->getW();
		_Parent->setW (clippedNewW);
		return dx;
	}

	// ***************************************************************************
	sint32 CCtrlResizer::resizeH (sint32 dy)
	{
		// if the owner is a container, special resize applied
		CGroupContainer *gc = NULL;
		gc = dynamic_cast<CGroupContainer *>(_Parent);
		if (gc == NULL)
			return 0;

		// resize popupmaxh or h, according to IsMaxH.
		sint32	oldH;
		if (IsMaxH)
			oldH= gc->getPopupMaxH();
		else
			oldH= _Parent->getH();

		// new H
		sint32	clippedNewH= oldH + dy;
		// if IsMaxH, don't clamp by HMax
		if (IsMaxH)
			clippedNewH = std::max(clippedNewH, HMin);
		else
			NLMISC::clamp(clippedNewH, HMin, HMax);


		// clip by screen
		uint32 sw, sh;
		CViewRenderer &vr = *CViewRenderer::getInstance();
		vr.getScreenSize(sw, sh);
		if (_Parent->getPosRef() & Hotspot_Tx)
		{
			if (_ResizerPos & Hotspot_Tx)
			{
				clippedNewH = std::min((sint32) sh + oldH - _Parent->getY(), clippedNewH);
			}
			else
			{
				clippedNewH = std::min(clippedNewH, _Parent->getY());
			}
		}
		else
		{
			if (_ResizerPos & Hotspot_Tx)
			{
				clippedNewH = std::min((sint32) sh - _Parent->getY(), clippedNewH);
			}
			else
			{
				clippedNewH = std::min(clippedNewH, _Parent->getY() +  oldH);
			}
		}

		// set final result
		dy = clippedNewH - oldH;
		if (IsMaxH)
			gc->setPopupMaxH(clippedNewH);
		else
			gc->setH(clippedNewH);
		return dy;
	}


	// ***************************************************************************
	// CCtrlMover
	// ***************************************************************************

	// ***************************************************************************
	CCtrlMover::CCtrlMover(const TCtorParam &param, bool canMove, bool canOpen)
	: CCtrlBase(param)
	{
		_Moving= false;
		_CanMove = canMove;
		_CanOpen = canOpen;
		_HasMoved = false;
		_MovingInParentList  = false;
		_ParentScrollingUp   = false;
		_ParentScrollingDown = false;
		_WaitToOpenClose = false;
	}

	// ***************************************************************************
	CCtrlMover::~CCtrlMover()
	{
	}

	// ***************************************************************************
	COptionsContainerInsertion *CCtrlMover::getInsertionOptions()
	{
		static	NLMISC::CRefPtr<COptionsContainerInsertion> insertionOptions;
		if (insertionOptions) return insertionOptions;
		insertionOptions = (COptionsContainerInsertion *) CWidgetManager::getInstance()->getOptions("container_insertion_opt");
		return insertionOptions;
	}


	// ***************************************************************************
	void CCtrlMover::draw ()
	{
		#ifdef DRAW_GC_TEST_QUADS
			drawGCTestQuad(_RenderLayer, _XReal, _YReal, _WReal, _HReal, CRGBA(255, 0, 0, 127));
		#endif

		// No Op if window is minimized
		if(CViewRenderer::getInstance()->isMinimized())
			return;

		// draw insertion position if moving in parent list
		if (_MovingInParentList)
		{
			COptionsContainerInsertion *options = getInsertionOptions();
			if (!options) return;
			CViewRenderer &rVR = *CViewRenderer::getInstance();
			sint32 oldSciX, oldSciY, oldSciW, oldSciH;
			rVR.getClipWindow (oldSciX, oldSciY, oldSciW, oldSciH);
			uint32 sw, sh;
			rVR.getScreenSize(sw, sh);
			rVR.setClipWindow (0, 0, (sint32) sw, (sint32) sh);
			CViewRenderer &vr = *CViewRenderer::getInstance();
			//
			CGroupContainer *gc = dynamic_cast<CGroupContainer *>(_Parent);
			if (!gc) return;
			CGroupList *gl = gc->getPreviousParentList();
			if (!gl) return;
			//
			sint32 arrowWidth, arrowHeight;
			//
			if (_ParentScrollingUp)
			{
				sint32 topPosY = gl->getChild(0)->getYReal();
				if (gc->getYReal() < topPosY)
				{
					vr.getTextureSizeFromId(options->TxId_T_Arrow, arrowWidth, arrowHeight);
					// insertion position is too high, just draw an arrow pointing to top
					sint32 px = gl->getXReal() + (gl->getWReal() >> 1) - (arrowWidth >> 1);
					vr.drawRotFlipBitmap(gc->getRenderLayer(), px, _ParentListTop - arrowHeight - 2, arrowWidth, arrowHeight, 0, 0, options->TxId_T_Arrow);
				}
			}
			//
			if (_ParentScrollingDown)
			{
				sint32 bottomPosY = gl->getChild(gl->getNumChildren() - 1)->getYReal() - gl->getChild(gl->getNumChildren() - 1)->getHReal();
				if (gc->getYReal() -  gc->getHReal() > bottomPosY)
				{
					vr.getTextureSizeFromId(options->TxId_B_Arrow, arrowWidth, arrowHeight);
					// draw an arrow pointing at bottom
					// insertion position is too high, just draw an arrow pointing to top
					sint32 px = gl->getXReal() + (gl->getWReal() >> 1) - (arrowWidth >> 1);
					vr.drawRotFlipBitmap(gc->getRenderLayer(), px, _ParentListBottom + 2, arrowWidth, arrowHeight, 0, 0, options->TxId_B_Arrow);
				}
			}


			if (!_ParentScrollingUp && !_ParentScrollingDown)
			{
				sint32 posY;
				if (_InsertionIndex == (sint32) gl->getNumChildren())
				{
					posY = gl->getChild(_InsertionIndex - 1)->getYReal();
				}
				else
				{
					posY = gl->getChild(_InsertionIndex)->getYReal() + gl->getChild(_InsertionIndex)->getHReal();

				}
				// draw insertion bar
				//
				sint32 barWidth, barHeight;
				vr.getTextureSizeFromId(options->TxId_InsertionBar, barWidth, barHeight);
				if (posY >= _ParentListBottom && posY <= _ParentListTop)
				{
					sint32 py = posY - (barHeight >> 1) - 3;
					vr.drawRotFlipBitmap(gc->getRenderLayer(), gl->getXReal(), py, gl->getWReal(), barHeight, 0, 0, options->TxId_InsertionBar);
				}
			}
			rVR.setClipWindow(oldSciX, oldSciY, oldSciW, oldSciH);
		}
	}

	// ***************************************************************************
	bool CCtrlMover::handleEvent (const NLGUI::CEventDescriptor &event)
	{
		if (CCtrlBase::handleEvent(event)) return true;
		if (!_Active)
			return false;

		const CWidgetManager::SInterfaceTimes &times = CWidgetManager::getInstance()->getInterfaceTimes();

		if (event.getType() == NLGUI::CEventDescriptor::system)
		{
			const NLGUI::CEventDescriptorSystem &eds = (const NLGUI::CEventDescriptorSystem &) event;
			if (eds.getEventTypeExtended() == NLGUI::CEventDescriptorSystem::setfocus)
			{
				const NLGUI::CEventDescriptorSetFocus &edsf = (const NLGUI::CEventDescriptorSetFocus &) eds;
				if (edsf.hasFocus() == false && _Moving)
				{
					stopMove();
					return true;
				}
			}
		}

		if (event.getType() == NLGUI::CEventDescriptor::mouse)
		{
			const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;
			// the ctrl must have been captured
			if (CWidgetManager::getInstance()->getCapturePointerLeft() != this)
				return false;

			CGroupContainer *gc = dynamic_cast<CGroupContainer *>(_Parent);
			if (!gc) return false;
			if (gc->isLockable())
			{
				if (gc->isLocked())
				{
					return false; // do nothing
				}
			}

			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftdown && _WaitToOpenClose)
			{
				if (_WaitToOpenClose)
				{
					_WaitToOpenClose = false;
					CWidgetManager::getInstance()->unregisterClockMsgTarget(this);
					// _WaitToOpen can only be set if the container is popable
					if (gc)
					{
						// A double click has been made
						gc->setHighLighted(false);
						if (gc->isPopuped())
						{
							// pop in the window
							gc->setPopupX(gc->getX());
							gc->setPopupY(gc->getY());
							gc->setPopupW(gc->getW());
							//sint32 currH, minH, maxH;
							//gc->getResizableChildrenH(currH, minH, maxH);
							//gc->setPopupChildrenH(currH);
							//
							gc->popin();
						}
						else
						{
							// pop the window
							gc->popupCurrentPos();
							gc->forceRolloverAlpha();
							if (gc->getPopupW() != -1)
							{
								gc->setX(gc->getPopupX());
								gc->setY(gc->getPopupY());
								gc->setW(gc->getPopupW());
								// must resize the children to get correct height
								//gc->setChildrenH(gc->getPopupChildrenH());
							}
							else
							{
								gc->setW(gc->getRefW());
							}
						}
						gc->invalidateCoords(2);
						//
						CWidgetManager::getInstance()->setCapturePointerLeft(NULL);
						CWidgetManager::getInstance()->setCapturePointerRight(NULL);
					}
					return true;
				}
			}

			if (_WaitToOpenClose)
			{
				_WaitToOpenClose = false;
				CWidgetManager::getInstance()->unregisterClockMsgTarget(this);
			}

			if (_CanOpen || gc->isOpenWhenPopup())
			{
				if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup)
				{
					if (!_Parent) return false;
					gc->setHighLighted(false);
					if (_HasMoved || _MovingInParentList)
					{
						stopMove();
						return true;
					}
					if (isIn(eventDesc.getX(), eventDesc.getY()))
					{

						if (gc->isPopable())
						{
							_WaitToOpenClose = true;
							CWidgetManager::getInstance()->registerClockMsgTarget(this);
							_WaitToOpenCloseDate = times.thisFrameMs;
						}
						else
						{
							_Moving = false;
							if (gc->isOpenable() && !gc->isOpenWhenPopup())
							{
								gc->setOpen(!gc->isOpen());
							}
							else
							{
								return runTitleActionHandler();
							}
						}
						_Moving = false;
						return true;
					}
					else
					{
						return false;
					}
				}
			}


			// Move Window Mgt.
			if(!_Moving && !_MovingInParentList)
			{
				if (_CanMove)
				{
					// Enter Moving?
					if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftdown )
					{
						_MoveStartX= _Parent->getX()-eventDesc.getX();
						_MoveStartY= _Parent->getY()-eventDesc.getY();
						_MoveDeltaXReal= _Parent->getXReal() - _Parent->getX();
						_MoveDeltaYReal= _Parent->getYReal() - _Parent->getY();
						_Moving= true;
						// set the window at top.
						CWidgetManager::getInstance()->setTopWindow(_Parent);
						if (gc->getAHOnBeginMovePtr())
						{
							CAHManager::getInstance()->runActionHandler(gc->getAHOnBeginMove(), gc, gc->getAHOnBeginMoveParams());						
						}
						return true;
					}
				}
			}
			else
			{
				// Leave Moving?
				if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup )
				{
					stopMove();
					return true;
				}
				// Move
				if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousemove )
				{
					_HasMoved = true;
					if (gc) gc->touch();
					// new coords
					sint32	x= eventDesc.getX() + _MoveStartX;
					sint32	y= eventDesc.getY() + _MoveStartY;

					// if the father is a container and is popable (but not poped), move it only if the user has moved the mouse enough
					if (_Parent && !_MovingInParentList)
					{
						if (gc && gc->getLayerSetup() != 0)
						{
							if (gc->isMovableInParentList() && !gc->isPopable())
							{
								if (abs(y - _Parent->getY()) > DELTA_BEFORE_MOVING_IN_PARENT_LIST)
								{
									// There should be at least one other entry in the parent list
									CGroupList *parentList = dynamic_cast<CGroupList *>(gc->getParent());
									if (!parentList) return false;
									if (parentList->getNbElement() == 1) return false;
									setMovingInParent(gc, x, y, eventDesc);
									updateInsertionIndex(parentList, eventDesc.getY());
									return true;
								}
							}

							if (gc->isPopable())
							{
								if (!gc->isMovableInParentList())
								{
									if (abs(x - _Parent->getX()) > DELTA_BEFORE_POPUP || abs(y - _Parent->getY()) > DELTA_BEFORE_POPUP)
									{
										setPoped(gc, x, y, eventDesc);
										return true;
									}
								}
								else
								{
									if (abs(y - _Parent->getY()) > DELTA_BEFORE_MOVING_IN_PARENT_LIST)
									{
										// There should be at least one other entry in the parent list
										CGroupList *parentList = dynamic_cast<CGroupList *>(gc->getParent());
										if (!parentList) return false;
										if (parentList->getNbElement() == 1) return false;
										setMovingInParent(gc, x, y, eventDesc);
										updateInsertionIndex(parentList, eventDesc.getY());
										return true;
									}
									else // the mouse should move on the side of the container to turn it into a popup
									//if (_Parent->getX() - x > DELTA_BEFORE_POPUP || x - (_Parent->getX() + _Parent->getWReal()) > DELTA_BEFORE_POPUP)
									if (abs(x - _Parent->getX()) > DELTA_BEFORE_POPUP)
									{
										setPoped(gc, x, y, eventDesc);
										return true;
									}
								}
							}

							if (abs(x - _Parent->getX()) > 1 || abs(y - _Parent->getY()) > 1)
							{
								gc->setHighLighted(true);
								return true;
							}
							else
							{
								gc->setHighLighted(false);
								return true; // mouse has not moved enough
							}
						}
					}

					if (!_MovingInParentList)
					{
						// clip, in real coords space
						uint32	wScreen, hScreen;
						CViewRenderer::getInstance()->getScreenSize(wScreen, hScreen);
						x+= _MoveDeltaXReal;
						y+= _MoveDeltaYReal;

						clamp(x, 0, (sint32)wScreen-_Parent->getWReal());
						clamp(y, 0, (sint32)hScreen-_Parent->getHReal());
						x-= _MoveDeltaXReal;
						y-= _MoveDeltaYReal;
						// move window
						_Parent->setX(x);
						_Parent->setY(y);

						// if some action handler to call when moving
						if(gc->getAHOnMovePtr())
						{
							// udpate XReal/YReal coords only of the container
							gc->CInterfaceElement::updateCoords();
							// execute the AH
							CAHManager::getInstance()->runActionHandler(gc->getAHOnMovePtr(), this, gc->getAHOnMoveParams());
						}
					}
					else
					{
						if (!gc) return false;
						const CGroupList *gl = gc->getPreviousParentList();
						if (gl)
						{
							updateInsertionIndex(gl, eventDesc.getY());
							// compute visible portion of list
							sint32 glSciX, glSciY, glSciW, glSciH;
							gl->getClip(glSciX, glSciY, glSciW, glSciH);
							_ParentListTop    = glSciY + glSciH;
							_ParentListBottom = glSciY;
							// the control is moving in its parent list, so the x coordinate doesn't change
							y += _MoveDeltaYReal;
							// if the group is at the bottom of screen or at the bottom of the list, must clamp & scroll down
							if (y < _ParentListBottom)
							{
								if (_ParentScrollingUp)
								{
									_ParentScrollingUp = false;
									CWidgetManager::getInstance()->registerClockMsgTarget(this); // want to now when time pass
								}
								if (glSciY > gl->getYReal()) // is there need for scroll ?
								{
									if (!_ParentScrollingDown)
									{
										_ParentScrollingDown = true;
										CWidgetManager::getInstance()->registerClockMsgTarget(this); // want to now when time pass
										_ScrollTime = 0;
									}
								}
								else
								{
									if (_ParentScrollingDown)
									{
										_ParentScrollingDown = false;
										CWidgetManager::getInstance()->unregisterClockMsgTarget(this); // want to now when time pass
									}
								}
								y = _ParentListBottom;
							}
							else
							{
								if (_ParentScrollingDown)
								{
									_ParentScrollingDown = false;
									CWidgetManager::getInstance()->registerClockMsgTarget(this); // want to now when time pass
								}
								sint32 topY = y + _Parent->getHReal();
								if (topY > _ParentListTop)
								{
									// idem for top
									if (glSciY + glSciH < gl->getYReal() + gl->getHReal()) // is there need for scroll ?
									{
										if (!_ParentScrollingUp)
										{
											_ParentScrollingUp = true;
											CWidgetManager::getInstance()->registerClockMsgTarget(this); // want to now when time pass
											_ScrollTime = 0;
										}
									}
									else
									{
										if (_ParentScrollingUp)
										{
											_ParentScrollingDown = false;
											CWidgetManager::getInstance()->unregisterClockMsgTarget(this); // want to now when time pass
										}
									}
									y = _ParentListTop - _Parent->getHReal();
								}
							}
							y -= _MoveDeltaYReal;
							// move window
							_Parent->setY(y);
						}
					}
					// just invalidate position (1 pass)
					_Parent->invalidateCoords(1);
					return true;
				}
			}
		}
		if (event.getType() == NLGUI::CEventDescriptor::system)
		{
			const NLGUI::CEventDescriptorSystem &systemEvent = (const NLGUI::CEventDescriptorSystem &) event;
			if (systemEvent.getEventTypeExtended() == NLGUI::CEventDescriptorSystem::clocktick)
			{
				if (_WaitToOpenClose)
				{
					uint dbclickDelay = CWidgetManager::getInstance()->getUserDblClickDelay();
					if ((times.thisFrameMs - _WaitToOpenCloseDate) > dbclickDelay)
					{
						CGroupContainer *gc = dynamic_cast<CGroupContainer *>(_Parent);
						if (!gc) return false;
						_WaitToOpenClose = false;
						CWidgetManager::getInstance()->unregisterClockMsgTarget(this);
						// do the open action
						if (gc->isOpenable() && !gc->isOpenWhenPopup())
						{
							gc->setOpen(!gc->isOpen());
						}
						else
						{
							// if can't open, just call the action handler
							return runTitleActionHandler();
						}
					}
				}
				else if (_ParentScrollingDown || _ParentScrollingUp)
				{
					handleScrolling();
				}
			}
		}
		return false;
	}

	// ***************************************************************************
	void CCtrlMover::handleScrolling()
	{
		const uint pixPerMS = 7; // the number of millisecond to move of one pixel in the parent scrollbar
		CGroupContainer *gc = dynamic_cast<CGroupContainer *>(_Parent);
		if (!gc) return;
		CGroupList *gl = gc->getPreviousParentList();
		if (!gl) return;
		
		const CWidgetManager::SInterfaceTimes &times = CWidgetManager::getInstance()->getInterfaceTimes();

		if (_ParentScrollingUp)
		{
			sint32 topPosY = gl->getChild(0)->getYReal();
			// check if we are really at the end of the list, if this is not the case, we should perform scroll on parent container with a scroll bar
			if (gc->getYReal() < topPosY)
			{
				_ScrollTime += times.frameDiffMs;
				sint32 deltaY = (sint32) (_ScrollTime / pixPerMS);
				if (deltaY != 0)
				{
					CGroupContainer *currGC = gc->getPreviousContainer();
					while (currGC)
					{
						CCtrlScroll *cs = currGC->getScroll();
						if (cs)
						{
							sint32 dy = cs->moveTrackY(deltaY);
							if (dy != 0) break;
						}//
						currGC = currGC->getFatherContainer();
					}
					gl->invalidateCoords();
					gc->invalidateCoords();
					_ScrollTime = _ScrollTime % (sint64) pixPerMS;
				}
			}
			else
			{
				_ParentScrollingUp = false;
				CWidgetManager::getInstance()->unregisterClockMsgTarget(this);
				_InsertionIndex = 0;
			}
		}
		//
		if (_ParentScrollingDown)
		{
			// check if we are really at the end of the list, if this is not the case, we should perform scroll on parent container with a scroll bar
			sint32 bottomPosY = gl->getChild(gl->getNumChildren() - 1)->getYReal() - gl->getChild(gl->getNumChildren() - 1)->getHReal();
			if (gc->getYReal() -  gc->getHReal() > bottomPosY)
			{
				_ScrollTime += times.frameDiffMs;
				sint32 deltaY = - (sint32) (_ScrollTime / pixPerMS);
				if (deltaY != 0)
				{
					CGroupContainer *currGC = gc->getPreviousContainer();
					while (currGC)
					{
						CCtrlScroll *cs = currGC->getScroll();
						if (cs)
						{
							sint32 dy = cs->moveTrackY(deltaY);
							if (dy != 0) break;
						}
						currGC = currGC->getFatherContainer();
					}
					gl->invalidateCoords();
					gc->invalidateCoords();
					_ScrollTime = _ScrollTime % pixPerMS;
				}
			}
			else
			{
				_ParentScrollingDown = false;
				CWidgetManager::getInstance()->unregisterClockMsgTarget(this);
				_InsertionIndex = gl->getNumChildren();
			}
		}
	}

	// ***************************************************************************
	bool CCtrlMover::runTitleActionHandler()
	{
		CGroupContainer *gc = dynamic_cast<CGroupContainer *>(_Parent);
		if (!gc) return false;
		CInterfaceGroup *gr = gc->isOpen() ? gc->getHeaderOpened() : gc->getHeaderClosed();
		if (gr && !gr->getLeftClickHandler().empty())
		{
			CAHManager::getInstance()->runActionHandler(gr->getLeftClickHandler(), this, gr->getLeftClickHandlerParams());
			return true;
		}
		// try with the main group
		if (!gc->getLeftClickHandler().empty())
		{
			CAHManager::getInstance()->runActionHandler(gc->getLeftClickHandler(), this, gc->getLeftClickHandlerParams());
				return true;
		}
		return false;
	}

	// ***************************************************************************
	void CCtrlMover::setPoped(CGroupContainer *gc, sint32 x, sint32 y, const NLGUI::CEventDescriptorMouse &eventDesc)
	{
		gc->setHighLighted(false);
		sint32 deltaX = x - _Parent->getX();
		sint32 deltaY = y - _Parent->getY();
		// pop up the window
		gc->popupCurrentPos(); // NB : this has the side effect of destroying this object during the call to CGroupContainer::update(), because the mover is recreated during the setup !
					// So from now we shouldn't use anything that use the 'this' pointer
					// TODO : maybe there a more clean way to do that ? (except that we may not call update() in popupCurrentPos() )
		if (gc->isLockable())
		{
			gc->setLocked(false);
		}
		gc->setW(gc->getRefW());
		gc->updateCoords();
		gc->updateCoords();
		gc->updateCoords();
		// now the window is in screen coordinates
		sint32 newX = gc->getXReal() + deltaX;
		sint32 newY = gc->getYReal() + deltaY;
		uint32	wScreen, hScreen;
		CViewRenderer::getInstance()->getScreenSize(wScreen, hScreen);
		clamp(newX, 0, (sint32)wScreen - gc->getWReal());
		clamp(newY, 0, (sint32)hScreen - gc->getHReal());
		// move window
		gc->setX(newX);
		gc->setY(newY + gc->getHReal());
		// just invalidate position (1 pass)
		gc->updateCoords();

		// delegate to the new created control mover
		CCtrlMover *cm = gc->getCtrlMover();
		cm->_MoveStartX= gc->getX()-eventDesc.getX();
		cm->_MoveStartY= gc->getY()-eventDesc.getY();
		cm->_MoveDeltaXReal= gc->getXReal() - gc->getX();
		cm->_MoveDeltaYReal= gc->getYReal() - gc->getY();
		cm->_Moving= true;
		CWidgetManager::getInstance()->setCapturePointerLeft(cm);
		CWidgetManager::getInstance()->setCapturePointerRight(NULL);
	}

	// ***************************************************************************
	void CCtrlMover::setMovingInParent(CGroupContainer *gc, sint32 /* x */, sint32 y, const NLGUI::CEventDescriptorMouse &eventDesc)
	{
		if (!gc) return;
		sint32 deltaY = y - gc->getY();
		CGroupList *parentList = dynamic_cast<CGroupList *>(gc->getParent());
		if (!parentList) return;
	//	sint32 startIndex = parentList->getElementIndex(gc);
		gc->setHighLighted(false);
		sint32 oldX = _Parent->getXReal();
		gc->setMovingInParentList(true);
		sint32 gcWidth = gc->getWReal();
		// pop up the window
		gc->popupCurrentPos(); // this doesn't change the order in setup
		gc->setSizeRef(0);
		gc->setW(gcWidth);
		gc->setX(oldX);
		gc->updateCoords();
		gc->updateCoords();
		gc->updateCoords();
		// now the window is in screen coordinates
		sint32 newY = gc->getYReal() + deltaY;
		//
		parentList->updateCoords();
		// get clip rect from parent list
		sint32 glSciX, glSciY, glSciW, glSciH;
		parentList->getClip(glSciX, glSciY, glSciW, glSciH);
		_ParentListTop    = glSciY + glSciH;
		_ParentListBottom = glSciY;
		// clip by parent list coords
		clamp(newY, _ParentListBottom, _ParentListTop - gc->getHReal());
		// move window
		gc->setY(newY + gc->getHReal());
		// just invalidate position (1 pass)
		gc->updateCoords();

		// reupdate pos
		_MoveStartY= gc->getY()-eventDesc.getY();
		_MoveDeltaYReal= gc->getYReal() - gc->getY();

		CWidgetManager::getInstance()->setCapturePointerLeft(this);
		CWidgetManager::getInstance()->setCapturePointerRight(NULL);
		_Moving = false;
		_MovingInParentList = true;

		// register to get time events
	}

	// ***************************************************************************
	void CCtrlMover::updateInsertionIndex(const CGroupList *gl, sint32 posY)
	{
		if (!gl) return;
		for(uint k = 0; k < gl->getNumChildren(); ++k)
		{
			CViewBase *child = gl->getChild(k);
			if (child->getYReal() <= posY)
			{
				if (posY < child->getYReal() + (child->getHReal() >> 1))
				{
					_InsertionIndex = k + 1;
					return;
				}
				else
				{
					_InsertionIndex = k;
					return;
				}
			}
		}
		_InsertionIndex = gl->getNumChildren();
	}

	// ***************************************************************************
	void CCtrlMover::stopMove()
	{
		_ParentScrollingUp = false;
		_ParentScrollingDown = false;
		CWidgetManager::getInstance()->setCapturePointerLeft(NULL);
		_HasMoved = false;
		if (_Moving)
		{
			_Moving = false;
		}
		else
		{
			_MovingInParentList = false;
			// insert at good position in the parent list
			CGroupContainer *gc = dynamic_cast<CGroupContainer *>(_Parent);
			if (!gc) return;
			gc->popin(_InsertionIndex);
			if (gc->getChildrenObs())
			{
				gc->getChildrenObs()->childrenMoved(_StartIndex, _InsertionIndex, gc);
			}
		}
	}


	// ***************************************************************************
	// CGroupContainer
	// ***************************************************************************

	NLMISC_REGISTER_OBJECT(CViewBase, CGroupContainer, std::string, "container");

	// ***************************************************************************
	CGroupContainer::CGroupContainer(const TCtorParam &param)
	: CGroupContainerBase(param)
	{
		// faster than a virual call
		_IsGroupContainer = true;

		_CurrentRolloverAlphaContainer = 0.f;
		_CurrentRolloverAlphaContent = 0.f;

		_LayerSetup = -1;
		_Localize = true;
		_Content = NULL;
		_HeaderOpened = NULL;
		_HeaderClosed = NULL;
		_TitleOpened = NULL;
		_TitleClosed = NULL;
		_TitleDeltaMaxW = 0;
		_ViewOpenState = NULL;
		_RightButton = NULL;
		_HelpButton = NULL;
		_List = NULL;
		_ScrollBar = NULL;
		_Mover= NULL;
		_OldFatherContainer = NULL;
		_InsertionOrder = 0;
		_MinW = 222;
		_MaxW = 320;
		_BackupX = 0;
		_BackupY = 0;
		_PopupMinW = 222;
		_PopupMaxW = 500;
		_PopupMinH = 48;
		_PopupMaxH = 500;

		_BlinkDT = 0;
		_ChildrenObs = NULL;
		_NumBlinks = 0;

		_PopupX = -1;
		_PopupY = -1;
		_PopupW = -1;

		_RefW = 0;

		_Openable			= true;
		_Opened				= false;
		_OpenWhenPopup		= false;
		_OpenAtStart		= false;
		_OpenedBeforePopup	= false;

		_Lockable			= true;

		_EnabledResizer			= true;
		_ResizerTopSize			= -1;
		_Movable				= false;
		_MovableInParentList	= false;
		_Popable				= false;
		_Poped					= false;
		_HighLighted			= false;
		_Blinking				= false;
		_BlinkState				= false;
		_MovingInParentList		= false;
		_ActiveSavable			= true;
		_Savable                = true;
		_TitleClass				= TitleText;
		_TouchFlag				= false;
		_PositionBackuped       = false;
		_Modal					= false;

		_HeaderActive = true;
		_EnabledRightButton = true;
		_EnabledHelpButton = true;
		_TitleOverExtendViewText = false;


		// action handler
		_AHOnOpen = NULL;
		_AHOnClose = NULL;
		_AHOnCloseButton = NULL;
		_AHOnMove = NULL;
		_AHOnDeactiveCheck = NULL;
		_AHOnResize = NULL;
		_AHOnAlphaSettingsChanged = NULL;
		_AHOnBeginMove = NULL;

		std::fill(_Resizer, _Resizer + NumResizers, (CCtrlResizer *) 0);

		_ContentYOffset = 0;
	}

	// ***************************************************************************
	CGroupContainer::~CGroupContainer()
	{
	}

	// ***************************************************************************
	CGroupContainer::TTileClass	CGroupContainer::convertTitleClass(const char *ptr)
	{
		if(nlstricmp(ptr, "formated")==0)
			return TitleTextFormated;
		if(nlstricmp(ptr, "text_id")==0)
			return TitleTextId;
		if(nlstricmp(ptr, "text_dyn_string")==0)
			return TitleTextDynString;
		// default
		return TitleText;
	}

	std::string CGroupContainer::getProperty( const std::string &name ) const
	{
		if( name == "localize" )
		{
			return toString( _Localize );
		}
		else
		if( name == "title_class" )
		{
			switch( _TitleClass )
			{
			case TitleTextFormated:
				return "formated";
				break;

			case TitleTextId:
				return "text_id";
				break;

			case TitleTextDynString:
				return "text_dyn_string";
				break;
			}

			return "text";
		}
		else
		if( name == "content_y_offset" )
		{
			return toString( _ContentYOffset );
		}
		else
		if( name == "title" )
		{
			if( _TitleTextOpened == _TitleTextClosed )
				return _TitleTextOpened.toString();
			else
				return "";
		}
		else
		if( name == "title_opened" )
		{
			return _TitleTextOpened.toString();
		}
		else
		if( name == "title_closed" )
		{
			return _TitleTextClosed.toString();
		}
		else
		if( name == "header_active" )
		{
			return toString( _HeaderActive );
		}
		else
		if( name == "header_color" )
		{
			if( _HeaderColor.getNodePtr() != NULL )
				return _HeaderColor.getNodePtr()->getFullName();
			else
				return "";
		}
		else
		if( name == "right_button" )
		{
			return toString( _EnabledRightButton );
		}
		else
		if( name == "help_button" )
		{
			return toString( _EnabledHelpButton );
		}
		else
		if( name == "movable" )
		{
			return toString( _Movable );
		}
		else
		if( name == "popable" )
		{
			return toString( _Popable );
		}
		else
		if( name == "lockable" )
		{
			return toString( _Lockable );
		}
		else
		if( name == "locked" )
		{
			return toString( _Locked );
		}
		else
		if( name == "openable" )
		{
			return toString( _Openable );
		}
		else
		if( name == "opened" )
		{
			return toString( _Opened );
		}
		else
		if( name == "modal" )
		{
			return toString( _Modal );
		}
		else
		if( name == "open_when_popup" )
		{
			return toString( _OpenWhenPopup );
		}
		else
		if( name == "resizer" )
		{
			return toString( _EnabledResizer );
		}
		else
		if( name == "resizer_top_size" )
		{
			return toString( _ResizerTopSize );
		}
		else
		if( name == "on_open" )
		{
			return getAHString( name );
		}
		else
		if( name == "on_open_params" )
		{
			return _AHOnOpenParams.toString();
		}
		else
		if( name == "on_close" )
		{
			return getAHString( name );
		}
		else
		if( name == "on_close_params" )
		{
			return _AHOnCloseParams.toString();
		}
		else
		if( name == "on_close_button" )
		{
			return getAHString( name );
		}
		else
		if( name == "on_close_button_params" )
		{
			return _AHOnCloseButtonParams.toString();
		}
		else
		if( name == "on_move" )
		{
			return getAHString( name );
		}
		else
		if( name == "on_move_params" )
		{
			return _AHOnMoveParams.toString();
		}
		else
		if( name == "on_deactive_check" )
		{
			return getAHString( name );
		}
		else
		if( name == "on_deactive_check_params" )
		{
			return _AHOnDeactiveCheckParams.toString();
		}
		else
		if( name == "on_resize" )
		{
			return getAHString( name );
		}
		else
		if( name == "on_resize_params" )
		{
			return _AHOnResizeParams.toString();
		}
		else
		if( name == "on_alpha_settings_changed" )
		{
			return getAHString( name );
		}
		else
		if( name == "on_alpha_settings_changed_params" )
		{
			return _AHOnAlphaSettingsChangedParams.toString();
		}
		else
		if( name == "on_begin_move" )
		{
			return getAHString( name );
		}
		else
		if( name == "on_begin_move_params" )
		{
			return _AHOnBeginMoveParams.toString();
		}
		else
		if( name == "max_w" )
		{
			return toString( _MaxW );
		}
		else
		if( name == "min_w" )
		{
			return toString( _MinW );
		}
		else
		if( name == "pop_max_w" )
		{
			return toString( _PopupMaxW );
		}
		else
		if( name == "pop_min_w" )
		{
			return toString( _PopupMinW );
		}
		else
		if( name == "pop_max_h" )
		{
			return toString( _PopupMaxH );
		}
		else
		if( name == "pop_min_h" )
		{
			return toString( _PopupMinH );
		}
		else
		if( name == "movable_in_parent_list" )
		{
			return toString( _MovableInParentList );
		}
		else
		if( name == "savable" )
		{
			return toString( _Savable );
		}
		else
		if( name == "active_savable" )
		{
			return toString( _ActiveSavable );
		}
		else
		if( name == "modal_parent" )
		{
			return _ModalParentNames;
		}
		else
		if( name == "options" )
		{
			return _OptionsName;
		}
		else
		if( name == "title_delta_max_w" )
		{
			return toString( _TitleDeltaMaxW );
		}
		else
		if( name == "title_over_extend_view_text" )
		{
			return toString( _TitleOverExtendViewText );
		}
		else
		if( name == "help_page" )
		{
			return _HelpPage.toString();
		}
		else
			return CInterfaceGroup::getProperty( name );
	}


	void CGroupContainer::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "localize" )
		{
			bool b;
			if( fromString( value, b ) )
				_Localize = b;
			return;
		}
		else
		if( name == "title_class" )
		{
			if( value == "formated" )
				_TitleClass = TitleTextFormated;
			else
			if( value == "text_id" )
				_TitleClass = TitleTextId;
			else
			if( value == "text_dyn_string" )
				_TitleClass = TitleTextDynString;
			else
				_TitleClass = TitleText;

			return;

		}
		else
		if( name == "content_y_offset" )
		{
			sint8 i;
			if( fromString( value, i ) )
				_ContentYOffset = i;
			return;
		}
		else
		if( name == "title" )
		{
			_TitleTextOpened = _TitleTextClosed = value;
			return;
		}
		else
		if( name == "title_opened" )
		{
			_TitleTextOpened = value;
			return;
		}
		else
		if( name == "title_closed" )
		{
			_TitleTextClosed = value;
			return;
		}
		else
		if( name == "header_active" )
		{
			bool b;
			if( fromString( value, b ) )
				_HeaderActive = b;
			return;
		}
		else
		if( name == "header_color" )
		{
			_HeaderColor.link( value.c_str() );
			return;
		}
		else
		if( name == "right_button" )
		{
			bool b;
			if( fromString( value, b ) )
				_EnabledRightButton = b;
			return;
		}
		else
		if( name == "help_button" )
		{
			bool b;
			if( fromString( value, b ) )
				_EnabledHelpButton = b;
			return;
		}
		else
		if( name == "movable" )
		{
			bool b;
			if( fromString( value, b ) )
				_Movable = b;
			return;
		}
		else
		if( name == "popable" )
		{
			bool b;
			if( fromString( value, b ) )
				_Popable = b;
			return;
		}
		else
		if( name == "lockable" )
		{
			bool b;
			if( fromString( value, b ) )
				_Lockable = b;
			return;
		}
		else
		if( name == "locked" )
		{
			bool b;
			if( fromString( value, b ) )
				_Locked = b;
			return;
		}
		else
		if( name == "openable" )
		{
			bool b;
			if( fromString( value, b ) )
				_Openable = b;
			return;
		}
		else
		if( name == "opened" )
		{
			bool b;
			if( fromString( value, b ) )
				_Opened = b;
			return;
		}
		else
		if( name == "modal" )
		{
			bool b;
			if( fromString( value, b ) )
				_Modal = b;
			return;
		}
		else
		if( name == "open_when_popup" )
		{
			bool b;
			if( fromString( value, b ) )
				_OpenWhenPopup = b;
			return;
		}
		else
		if( name == "resizer" )
		{
			bool b;
			if( fromString( value, b ) )
				_EnabledResizer = b;
			return;
		}
		else
		if( name == "resizer_top_size" )
		{
			sint8 i;
			if( fromString( value, i ) )
				_ResizerTopSize = i;
			return;
		}
		else
		if( name == "on_open" )
		{
			std::string dummy;
			_AHOnOpen = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( name, value );
			return;
		}
		else
		if( name == "on_open_params" )
		{
			_AHOnOpenParams = value;
			return;
		}
		else
		if( name == "on_close" )
		{
			std::string dummy;
			_AHOnClose = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( name, value );
			return;
		}
		else
		if( name == "on_close_params" )
		{
			_AHOnCloseParams = value;
			return;
		}
		else
		if( name == "on_close_button" )
		{
			std::string dummy;
			_AHOnCloseButton = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( name, value );
			return;
		}
		else
		if( name == "on_close_button_params" )
		{
			_AHOnCloseButtonParams = value;
			return;
		}
		else
		if( name == "on_move" )
		{
			std::string dummy;
			_AHOnMove = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( name, value );
			return;
		}
		else
		if( name == "on_move_params" )
		{
			_AHOnMoveParams = value;
			return;
		}
		else
		if( name == "on_deactive_check" )
		{
			std::string dummy;
			_AHOnDeactiveCheck  = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( name, value );
			return;
		}
		else
		if( name == "on_deactive_check_params" )
		{
			_AHOnDeactiveCheckParams = value;
			return;
		}
		else
		if( name == "on_resize" )
		{
			std::string dummy;
			_AHOnResize = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( name, value );
			return;
		}
		else
		if( name == "on_resize_params" )
		{
			_AHOnResizeParams = value;
			return;
		}
		else
		if( name == "on_alpha_settings_changed" )
		{
			std::string dummy;
			_AHOnAlphaSettingsChanged = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( name, value );
			return;
		}
		else
		if( name == "on_alpha_settings_changed_params" )
		{
			_AHOnAlphaSettingsChangedParams = value;
			return;
		}
		else
		if( name == "on_begin_move" )
		{
			std::string dummy;
			_AHOnBeginMove = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( name, value );
			return;
		}
		else
		if( name == "on_begin_move_params" )
		{
			_AHOnBeginMoveParams = value;
			return;
		}
		else
		if( name == "max_w" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_MaxW = i;
			return;
		}
		else
		if( name == "min_w" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_MinW = i;
			return;
		}
		else
		if( name == "pop_max_w" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_PopupMaxW = i;
			return;
		}
		else
		if( name == "pop_min_w" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_PopupMinW = i;
			return;
		}
		else
		if( name == "pop_max_h" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_PopupMaxH = i;
			return;
		}
		else
		if( name == "pop_min_h" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_PopupMinH = i;
			return;
		}
		else
		if( name == "movable_in_parent_list" )
		{
			bool b;
			if( fromString( value, b ) )
				_MovableInParentList = b;
			return;
		}
		else
		if( name == "savable" )
		{
			bool b;
			if( fromString( value, b ) )
				_Savable = b;
			return;
		}
		else
		if( name == "active_savable" )
		{
			bool b;
			if( fromString( value, b ) )
				_ActiveSavable = b;
			return;
		}
		else
		if( name == "modal_parent" )
		{
			_ModalParentNames = value;
			return;
		}
		else
		if( name == "options" )
		{
			_OptionsName = value;
			return;
		}
		else
		if( name == "title_delta_max_w" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_TitleDeltaMaxW = i;
			return;
		}
		else
		if( name == "title_over_extend_view_text" )
		{
			bool b;
			if( fromString( value, b ) )
				_TitleOverExtendViewText = b;
			return;
		}
		else
		if( name == "help_page" )
		{
			_HelpPage = value;
			return;
		}
		else
			CInterfaceGroup::setProperty( name, value );
	}

	xmlNodePtr CGroupContainer::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CInterfaceGroup::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "container" );
		xmlSetProp( node, BAD_CAST "localize", BAD_CAST toString( _Localize ).c_str() );

		std::string titleClass;
		switch( _TitleClass )
		{
		case TitleTextFormated:
			titleClass = "formated";
			break;

		case TitleTextId:
			titleClass = "text_id";
			break;

		case TitleTextDynString:
			titleClass = "text_dyn_string";
			break;

		default:
			titleClass = "text";
			break;
		}

		xmlSetProp( node, BAD_CAST "title_class", BAD_CAST titleClass.c_str() );
		xmlSetProp( node, BAD_CAST "content_y_offset", BAD_CAST toString( _ContentYOffset ).c_str() );
		
		if( _TitleTextOpened == _TitleTextClosed )
			xmlSetProp( node, BAD_CAST "title", BAD_CAST _TitleTextOpened.toString().c_str() );
		else
			xmlSetProp( node, BAD_CAST "title", BAD_CAST "" );

		xmlSetProp( node, BAD_CAST "title_opened", BAD_CAST _TitleTextOpened.toString().c_str() );
		xmlSetProp( node, BAD_CAST "title_closed", BAD_CAST _TitleTextClosed.toString().c_str() );
		xmlSetProp( node, BAD_CAST "header_active", BAD_CAST toString( _HeaderActive ).c_str() );

		if( _HeaderColor.getNodePtr() != NULL )
			xmlSetProp( node, BAD_CAST "header_color", BAD_CAST _HeaderColor.getNodePtr()->getFullName().c_str() );
		else
			xmlSetProp( node, BAD_CAST "header_color", BAD_CAST "" );

		xmlSetProp( node, BAD_CAST "right_button", BAD_CAST toString( _EnabledRightButton ).c_str() );
		xmlSetProp( node, BAD_CAST "help_button", BAD_CAST toString( _EnabledHelpButton ).c_str() );
		xmlSetProp( node, BAD_CAST "movable", BAD_CAST toString( _Movable ).c_str() );
		xmlSetProp( node, BAD_CAST "popable", BAD_CAST toString( _Popable ).c_str() );
		xmlSetProp( node, BAD_CAST "lockable", BAD_CAST toString( _Lockable ).c_str() );
		xmlSetProp( node, BAD_CAST "locked", BAD_CAST toString( _Locked ).c_str() );
		xmlSetProp( node, BAD_CAST "openable", BAD_CAST toString( _Openable ).c_str() );
		xmlSetProp( node, BAD_CAST "opened", BAD_CAST toString( _Opened ).c_str() );
		xmlSetProp( node, BAD_CAST "modal", BAD_CAST toString( _Modal ).c_str() );
		xmlSetProp( node, BAD_CAST "open_when_popup", BAD_CAST toString( _OpenWhenPopup ).c_str() );
		xmlSetProp( node, BAD_CAST "resizer", BAD_CAST toString( _EnabledResizer ).c_str() );
		xmlSetProp( node, BAD_CAST "resizer_top_size", BAD_CAST toString( _ResizerTopSize ).c_str() );
		
		xmlSetProp( node, BAD_CAST "on_open",
			BAD_CAST getAHString( "on_open" ).c_str()  );

		xmlSetProp( node, BAD_CAST "on_open_params",
			BAD_CAST _AHOnOpenParams.toString().c_str()  );

		xmlSetProp( node, BAD_CAST "on_close",
			BAD_CAST getAHString( "on_close" ).c_str()  );

		xmlSetProp( node, BAD_CAST "on_close_params",
			BAD_CAST _AHOnCloseParams.toString().c_str()  );

		xmlSetProp( node, BAD_CAST "on_close_button",
			BAD_CAST getAHString( "on_close_button" ).c_str()  );

		xmlSetProp( node, BAD_CAST "on_close_button_params",
			BAD_CAST _AHOnCloseButtonParams.toString().c_str()  );

		xmlSetProp( node, BAD_CAST "on_move",
			BAD_CAST getAHString( "on_move" ).c_str()  );

		xmlSetProp( node, BAD_CAST "on_close_params",
			BAD_CAST _AHOnMoveParams.toString().c_str()  );

		xmlSetProp( node, BAD_CAST "on_deactive_check",
			BAD_CAST getAHString( "on_deactive_check" ).c_str()  );

		xmlSetProp( node, BAD_CAST "on_deactive_check_params",
			BAD_CAST _AHOnDeactiveCheckParams.toString().c_str()  );

		xmlSetProp( node, BAD_CAST "on_resize",
			BAD_CAST getAHString( "on_resize" ).c_str()  );

		xmlSetProp( node, BAD_CAST "on_resize_params",
			BAD_CAST _AHOnResizeParams.toString().c_str()  );

		xmlSetProp( node, BAD_CAST "on_alpha_settings_changed",
			BAD_CAST getAHString( "on_alpha_settings_changed" ).c_str()  );

		xmlSetProp( node, BAD_CAST "on_alpha_settings_changed_params",
			BAD_CAST _AHOnAlphaSettingsChangedParams.toString().c_str()  );

		xmlSetProp( node, BAD_CAST "on_begin_move",
			BAD_CAST getAHString( "on_begin_move" ).c_str()  );

		xmlSetProp( node, BAD_CAST "on_begin_move_params",
			BAD_CAST _AHOnBeginMoveParams.toString().c_str()  );

		xmlSetProp( node, BAD_CAST "max_w", BAD_CAST toString( _MaxW ).c_str() );
		xmlSetProp( node, BAD_CAST "min_w", BAD_CAST toString( _MinW ).c_str() );
		xmlSetProp( node, BAD_CAST "pop_max_w", BAD_CAST toString( _PopupMaxW ).c_str() );
		xmlSetProp( node, BAD_CAST "pop_min_w", BAD_CAST toString( _PopupMinW ).c_str() );
		xmlSetProp( node, BAD_CAST "pop_max_h", BAD_CAST toString( _PopupMaxH ).c_str() );
		xmlSetProp( node, BAD_CAST "pop_min_h", BAD_CAST toString( _PopupMinH ).c_str() );
		xmlSetProp( node, BAD_CAST "movable_in_parent_list", BAD_CAST toString( _MovableInParentList ).c_str() );
		xmlSetProp( node, BAD_CAST "savable", BAD_CAST toString( _Savable ).c_str() );
		xmlSetProp( node, BAD_CAST "active_savable", BAD_CAST toString( _ActiveSavable ).c_str() );
		xmlSetProp( node, BAD_CAST "modal_parents", BAD_CAST _ModalParentNames.c_str() );
		xmlSetProp( node, BAD_CAST "options", BAD_CAST _OptionsName.toString().c_str() );
		xmlSetProp( node, BAD_CAST "title_delta_max_w", BAD_CAST toString( _TitleDeltaMaxW ).c_str() );
		xmlSetProp( node, BAD_CAST "title_over_extend_view_text", BAD_CAST toString( _TitleOverExtendViewText ).c_str() );
		xmlSetProp( node, BAD_CAST "help_page", BAD_CAST _HelpPage.toString().c_str() );

		return node;
	}


	xmlNodePtr CGroupContainer::serializeTreeData( xmlNodePtr parentNode ) const
	{
		xmlNodePtr node = CInterfaceGroup::serializeTreeData( parentNode );
		if( node == NULL )
			return NULL;

		if( _List == NULL )
			return NULL;

		CInterfaceGroup *g = NULL;
		for( sint32 i = 0; i < _List->getChildrenNb(); i++ )
		{
			g = dynamic_cast< CInterfaceGroup* >( _List->getChild( i ) );
			if( g == NULL )
				continue;
			
			if( g->serializeTreeData( node ) == NULL )
				return NULL;
		}

		return node;
	}

	// ***************************************************************************
	bool CGroupContainer::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
	{
		if (!CInterfaceGroup::parse(cur,parentGroup))
			return false;
		CXMLAutoPtr ptr;

		ptr = xmlGetProp (cur, (xmlChar*)"localize");
		if (ptr) _Localize = convertBool((const char*)ptr);

		// Type of the view text of the title
		ptr = xmlGetProp (cur, (xmlChar*)"title_class");
		if (ptr)
		{
			_TitleClass = convertTitleClass((const char*)ptr);
			// if textid, don't localize, because the title is a DB path
			if(_TitleClass==TitleTextId || _TitleClass==TitleTextDynString)
				_Localize= false;
		}

		// y offset for content
		ptr = xmlGetProp (cur, (xmlChar*)"content_y_offset");
		if (ptr)
		{
			fromString((const char*)ptr, _ContentYOffset);
		}

		// text of the title
		ptr = xmlGetProp (cur, (xmlChar*)"title");
		if (ptr)
		{
			if (_Localize)	_TitleTextOpened = CI18N::get(string((const char*)ptr));
			else			_TitleTextOpened = string((const char*)ptr);
			if (_Localize)	_TitleTextClosed = CI18N::get(string((const char*)ptr));
			else			_TitleTextClosed = string((const char*)ptr);
		}

		ptr = xmlGetProp (cur, (xmlChar*)"title_opened");
		if (ptr)
		{
			if (_Localize)	_TitleTextOpened = CI18N::get(string((const char*)ptr));
			else			_TitleTextOpened = string((const char*)ptr);
		}

		ptr = xmlGetProp (cur, (xmlChar*)"title_closed");
		if (ptr)
		{
			if (_Localize)	_TitleTextClosed = CI18N::get(string((const char*)ptr));
			else			_TitleTextClosed = string((const char*)ptr);
		}

		ptr = xmlGetProp (cur, (xmlChar*)"header_active");
		if (ptr)
			_HeaderActive = convertBool(ptr);

		ptr = xmlGetProp (cur, (xmlChar*)"header_color");
		if (ptr)
			_HeaderColor.link(ptr);

		ptr = xmlGetProp (cur, (xmlChar*)"right_button");
		if (ptr)
		{
			_EnabledRightButton = convertBool(ptr);
			// Default; take same state than the right button
			_EnabledHelpButton = _EnabledRightButton;
		}

		// but user may ovveride this case
		ptr = xmlGetProp (cur, (xmlChar*)"help_button");
		if (ptr)
			_EnabledHelpButton = convertBool(ptr);

		ptr = xmlGetProp (cur, (xmlChar*)"movable");
		if (ptr)
			_Movable = convertBool(ptr);

		ptr = xmlGetProp (cur, (xmlChar*)"popable");
		if (ptr)
			_Popable = convertBool(ptr);

		ptr = xmlGetProp (cur, (xmlChar*)"lockable");
		if (ptr)
			_Lockable = convertBool(ptr);

		ptr = xmlGetProp (cur, (xmlChar*)"locked");
		if (ptr)
			_Locked = convertBool(ptr);

		ptr = xmlGetProp (cur, (xmlChar*)"openable");
		if (ptr)
			_Openable = convertBool(ptr);

		ptr = xmlGetProp (cur, (xmlChar*)"opened");
		if (ptr)
			_OpenAtStart = convertBool(ptr);

		ptr = xmlGetProp (cur, (xmlChar*)"modal");
		if (ptr)
			_Modal = convertBool(ptr);

		ptr = xmlGetProp (cur, (xmlChar*)"open_when_popup");
		if (ptr)
		{
			_OpenWhenPopup = convertBool(ptr);
			if (_OpenWhenPopup)
				_OpenAtStart = false;
		}

		ptr = xmlGetProp (cur, (xmlChar*)"resizer");
		if (ptr)
			_EnabledResizer = convertBool(ptr);

		ptr = xmlGetProp (cur, (xmlChar*)"resizer_top_size");
		_ResizerTopSize= -1;
		if (ptr)
			fromString((const char*)ptr, _ResizerTopSize);

		CAHManager::getInstance()->parseAH(cur, "on_open", "on_open_params", _AHOnOpen, _AHOnOpenParams);
		CAHManager::getInstance()->parseAH(cur, "on_close", "on_close_params", _AHOnClose, _AHOnCloseParams);
		CAHManager::getInstance()->parseAH(cur, "on_close_button", "on_close_button_params", _AHOnCloseButton, _AHOnCloseButtonParams);
		CAHManager::getInstance()->parseAH(cur, "on_move", "on_move_params", _AHOnMove, _AHOnMoveParams);
		CAHManager::getInstance()->parseAH(cur, "on_deactive_check", "on_deactive_check_params", _AHOnDeactiveCheck, _AHOnDeactiveCheckParams);
		CAHManager::getInstance()->parseAH(cur, "on_resize", "on_resize_params", _AHOnResize, _AHOnResizeParams);
		CAHManager::getInstance()->parseAH(cur, "on_alpha_settings_changed", "on_alpha_settings_changed_params", _AHOnAlphaSettingsChanged, _AHOnAlphaSettingsChangedParams);
		CAHManager::getInstance()->parseAH(cur, "on_begin_move", "on_begin_move_params", _AHOnBeginMove, _AHOnBeginMoveParams);


		if( editorMode )
		{
			ptr = xmlGetProp( cur, BAD_CAST "on_open" );
			if( ptr )
				mapAHString( "on_open", std::string( (const char*)ptr ) );

			ptr = xmlGetProp( cur, BAD_CAST "on_close" );
			if( ptr )
				mapAHString( "on_close", std::string( (const char*)ptr ) );

			ptr = xmlGetProp( cur, BAD_CAST "on_close_button" );
			if( ptr )
				mapAHString( "on_close_button", std::string( (const char*)ptr ) );

			ptr = xmlGetProp( cur, BAD_CAST "on_move" );
			if( ptr )
				mapAHString( "on_move", std::string( (const char*)ptr ) );

			ptr = xmlGetProp( cur, BAD_CAST "on_deactive_check" );
			if( ptr )
				mapAHString( "on_deactive_check", std::string( (const char*)ptr ) );

			ptr = xmlGetProp( cur, BAD_CAST "on_resize" );
			if( ptr )
				mapAHString( "on_resize", std::string( (const char*)ptr ) );

			ptr = xmlGetProp( cur, BAD_CAST "on_alpha_settings_changed" );
			if( ptr )
				mapAHString( "on_alpha_settings_changed", std::string( (const char*)ptr ) );

			ptr = xmlGetProp( cur, BAD_CAST "on_begin_move" );
			if( ptr )
				mapAHString( "on_begin_move", std::string( (const char*)ptr ) );
		}

		ptr = xmlGetProp (cur, (xmlChar*)"max_w");
		if (ptr)
			fromString((const char*)ptr, _MaxW);
		ptr = xmlGetProp (cur, (xmlChar*)"min_w");
		if (ptr)
			fromString((const char*)ptr, _MinW);

		ptr = xmlGetProp (cur, (xmlChar*)"pop_max_w");
		if (ptr)
			fromString((const char*)ptr, _PopupMaxW);
		ptr = xmlGetProp (cur, (xmlChar*)"pop_min_w");
		if (ptr)
			fromString((const char*)ptr, _PopupMinW);

		ptr = xmlGetProp (cur, (xmlChar*)"pop_max_h");
		if (ptr)
			fromString((const char*)ptr, _PopupMaxH);
		ptr = xmlGetProp (cur, (xmlChar*)"pop_min_h");
		if (ptr)
			fromString((const char*)ptr, _PopupMinH);

		ptr = xmlGetProp (cur, (xmlChar*)"movable_in_parent_list");
		if (ptr) _MovableInParentList = convertBool(ptr);

		ptr = xmlGetProp (cur, (xmlChar*)"savable");
		if (ptr) _Savable = convertBool(ptr);

		ptr = xmlGetProp (cur, (xmlChar*)"active_savable");
		if (ptr) _ActiveSavable = convertBool(ptr);

		ptr = xmlGetProp (cur, (xmlChar*)"modal_parent");
		if (ptr) _ModalParentNames = (const char*)ptr;

		ptr = xmlGetProp (cur, (xmlChar*)"options");
		if (ptr) _OptionsName = (const char*)ptr;

		ptr = xmlGetProp (cur, (xmlChar*)"title_delta_max_w");
		if (ptr) fromString((const char*)ptr, _TitleDeltaMaxW);

		ptr = xmlGetProp (cur, (xmlChar*)"title_over_extend_view_text");
		_TitleOverExtendViewText= false;
		if (ptr) _TitleOverExtendViewText = convertBool(ptr);


		ptr = xmlGetProp (cur, (xmlChar*)"help_page");
		if (ptr) _HelpPage = (const char*)ptr;

		_RefW = _W;
		return true;
	}

	// ***************************************************************************
	void CGroupContainer::updateCoords()
	{
		if (_Mover && _Mover->isMovingInParentList())
		{
			// must clamp by parent coords, now it has been updated
			CGroupList *gl = getPreviousParentList();
			if (gl)
			{
				sint32 cx, cy, cw, ch;
				gl->getClip(cx, cy, cw, ch);
				sint32 currYReal = getYReal();
				clamp(currYReal, cy, cy + ch - getHReal());
				if (currYReal != getYReal())
				{
					setY(getY() + currYReal - getYReal());
				}
			}
		}

		setup();

		bool bHasChild = (_List->getNbElement() > 0);

		// clamp popupmaxh if resizemax mode
		if ((_LayerSetup == 0) && bHasChild && _EnabledResizer)
		{
			// Clip by screen
			uint32 sw, sh;
			CViewRenderer &vr = *CViewRenderer::getInstance();
			vr.getScreenSize(sw, sh);
			// ensure the maxH is > minH and < sh  (prioritary > minH, in case of sh<minh)
			_PopupMaxH= min(_PopupMaxH, (sint32)sh);
			_PopupMaxH= max(_PopupMaxH, _PopupMinH);
		}

		// clamp w/h if standard resizer
		if ((_LayerSetup == 0) && (!bHasChild) && _EnabledResizer)
		{
			clamp(_W, _PopupMinW, _PopupMaxW);
			clamp(_H, _PopupMinH, _PopupMaxH);
			// Clip by screen (but minw/minh is prioritary)
			uint32 sw, sh;
			CViewRenderer &vr = *CViewRenderer::getInstance();
			vr.getScreenSize(sw, sh);
			if((sint32)sw>_PopupMinW && _W>(sint32)sw)	_W= sw;
			if((sint32)sh>_PopupMinH && _H>(sint32)sh)	_H= sh;
		}

		COptionsLayer *pLayer = getContainerOptions();
		sint32 newH = 0;

		// Get base header size
		if (_LayerSetup == 0)
		{
			setMaxH(_PopupMaxH);
			// _W is given by scripter-man
			newH = (pLayer->H_T - pLayer->InsetT);
		}
		else
		{
			if (_SizeRef & 1)
			{
				_W = _Parent->getW();
			}
			setMaxH (16384); // No scrollbar for container of layer > 0
			newH = (pLayer->H_T - pLayer->InsetT);
		}

		if (_Opened)
		{
			if (_HeaderOpened != NULL)
			{
				if (_HeaderOpened->getPosRef()&Hotspot_xR)
					_HeaderOpened->setX (-pLayer->W_R);
				if (_HeaderOpened->getPosRef()&Hotspot_xL)
					_HeaderOpened->setX (pLayer->W_L);
				_HeaderOpened->setY (- newH);
				_HeaderOpened->setW (_W-(pLayer->W_L+pLayer->W_R));
				_HeaderOpened->updateCoords();
				newH += max (_HeaderOpened->getHReal(), pLayer->HeaderH);
			}
			else
			{
				newH += pLayer->HeaderH;
			}

			newH -= (sint32) _ContentYOffset;

			// Calculate content size part
			if (_Content != NULL)
			{
				if (_Content->getPosRef()&Hotspot_xR)
					_Content->setX (-pLayer->W_R);
				else if (_Content->getPosRef()&Hotspot_xL)
					_Content->setX (pLayer->W_L);
				_Content->setY (-newH);
				_Content->setW (-(pLayer->W_L+pLayer->W_R));
				if ((bHasChild) || (!_EnabledResizer) || (_LayerSetup>0)) // Content is constant in H
				{
					_Content->setSizeRef (1); // w
					_Content->updateCoords();
					newH += _Content->getHReal();
				}
				else
				{
					_Content->setSizeRef (3); // wh
					_Content->setH (-newH - pLayer->H_B); // Sub header and top, bottom bitmaps
				}
			}

			if (bHasChild)
				newH += pLayer->H_B_Open;
			else
				newH += pLayer->H_B;

			if (_LayerSetup == 0)
			{
				_List->setX (pLayer->W_M_Open);
				_ScrollBar->setX (pLayer->Scrollbar_Offset_X);
				_ScrollBar->setY(-newH);
			}
			else
			{
				_List->setX (0);
			}
			_List->setY (-newH);

			// Calculate list max height if top container
			if (_LayerSetup == 0)
			{
				// zeH is the height to substract to total height of the container to obtain height of the list
				sint32 zeH = (pLayer->H_T - pLayer->InsetT) + pLayer->H_B_Open + pLayer->H_EM_Open;

				if (_HeaderOpened != NULL)
					zeH += max (_HeaderOpened->getHReal(), pLayer->HeaderH);
				else
					zeH += pLayer->HeaderH;

				if (_Content != NULL)
					zeH += _Content->getHReal();

				if (_List != NULL)
					_List->setMaxH (max((sint32)0, _MaxH-zeH));
			}
			else
			{
				if (_List != NULL)
					_List->setMaxH (16384);
			}

			if (_LayerSetup == 0)
			{
				_List->forceSizeW(_W - pLayer->W_M_Open);
			}
			else
			{
				_List->forceSizeW(_W);
			}


			//CInterfaceElement::updateCoords();
			CInterfaceGroup::updateCoords();

			newH += ((_List->getHReal() < _List->getMaxH()) ? _List->getHReal() : _List->getMaxH());

			if (_LayerSetup == 0)
			{
				if ((!bHasChild) || (_List->getHReal() < _List->getMaxH()))
					_ScrollBar->setActive (false);
				else
					_ScrollBar->setActive (true);
			}

			if (_LayerSetup == 0)
			{
				if (_List->getNbElement() > 0)
				{
					newH += pLayer->H_EM_Open;
				}
			}

			if ((bHasChild) || (!_EnabledResizer) || (_LayerSetup>0)) // H is depending on the header and content and list
				_H = newH;

		}
		else // Closed
		{
			if (_HeaderClosed != NULL)
			{
				if (_HeaderClosed->getPosRef()&Hotspot_xR)
					_HeaderClosed->setX (-pLayer->W_R);
				else if (_HeaderClosed->getPosRef()&Hotspot_xL)
					_HeaderClosed->setX (pLayer->W_L);
				_HeaderClosed->setY (-newH);
				_HeaderClosed->setW (_W-(pLayer->W_L+pLayer->W_R));
				_HeaderClosed->updateCoords();
				newH += max (_HeaderClosed->getHReal(), pLayer->HeaderH);
			}
			else
			{
				newH += pLayer->HeaderH;
			}
			newH += pLayer->H_B;

			if ((bHasChild) || (!_EnabledResizer) || (_LayerSetup>0)) // H is depending on the header and content and list
				_H = newH;

			CInterfaceGroup::updateCoords();
		}


		if (_Mover != NULL)
		{
			_Mover->setW (_W+_MoverDeltaW);
			_Mover->updateCoords();
		}

		// Set MaxW for title according to current Container Width
		_TitleOpened->setLineMaxW(_W + _TitleDeltaMaxW);
		_TitleClosed->setLineMaxW(_W + _TitleDeltaMaxW);

		_TitleOpened->updateCoords();
		_TitleClosed->updateCoords();
		if (_ViewOpenState != NULL)	_ViewOpenState->updateCoords();
		if (_RightButton != NULL)	_RightButton->updateCoords();
		if (_HelpButton != NULL)	_HelpButton->updateCoords();
		if (_ScrollBar != NULL)		_ScrollBar->updateCoords();
		if (_Content != NULL)		_Content->updateCoords();
		if (_HeaderClosed != NULL)	_HeaderClosed->updateCoords();
		if (_HeaderOpened != NULL)	_HeaderOpened->updateCoords();

		CInterfaceElement::updateCoords();

		if (_LayerSetup == 0)
		{
			// test if must clip
			uint32	wScreen, hScreen;
			CViewRenderer::getInstance()->getScreenSize(wScreen, hScreen);
			if (_WReal <= (sint32) wScreen && _HReal <= (sint32) hScreen)
			{
				sint32 newX = _XReal;
				sint32 newY = _YReal;
				clamp(newX, 0, (sint32)wScreen - _WReal);
				clamp(newY, 0, (sint32)hScreen - _HReal);
				if (newX != _XReal || newY != _YReal)
				{
					setX(_X + newX - _XReal);
					setY(_Y + newY - _YReal);
					CInterfaceGroup::updateCoords();
				}
			}
		}

		// resizers
		for(uint k = 0; k < NumResizers; ++k)
		{
			if (_Resizer[k]) updateResizerSize(_Resizer[k]);
		}
	}

	// ***************************************************************************
	void CGroupContainer::updateResizerSize(CCtrlResizer *cr)
	{
		COptionsContainerMove *options = getMoveOptions();
		if (!options) return;

		// Yoyo: The +1 stuff is important, maybe because of the HotSpot MiddleMiddle style...

		// Choose H
		if (!(cr->getResizerPos() & Hotspot_Tx) &&  !(cr->getResizerPos() & Hotspot_Bx))
		{
			// if no special topH size
			if(_ResizerTopSize<0)
				cr->setH (_H - 2 * options->ResizerSize + 1);				// Bottom and Top same value
			else
				cr->setH (_H - options->ResizerSize - _ResizerTopSize + 1);	// Top different from bottom
		}
		else
		{
			// if no special topH size, or if the resizer is not the top one
			if(_ResizerTopSize<0 || !(cr->getResizerPos() & Hotspot_Tx))
				cr->setH(options->ResizerSize + 1);
			else
				cr->setH(_ResizerTopSize + 1);
		}

		// Choose W
		if (!(cr->getResizerPos() & Hotspot_xR) &&  !(cr->getResizerPos() & Hotspot_xL))
		{
			cr->setW (_W - 2 * options->ResizerSize + 1);
		}
		else
		{
			cr->setW(options->ResizerSize + 1);
		}

		// update coordinate
		cr->updateCoords();
	}

	// ***************************************************************************
	void CGroupContainer::draw ()
	{
		H_AUTO( RZ_Interface_CGroupContainer_draw  )

		if (_LayerSetup == -1) return;

		float speed = CWidgetManager::getInstance()->getAlphaRolloverSpeed();
		const CWidgetManager::SInterfaceTimes &times = CWidgetManager::getInstance()->getInterfaceTimes();

		CRGBA oldGlobalColor = CWidgetManager::getInstance()->getGlobalColor();
		CRGBA oldGColForGrayed = CWidgetManager::getInstance()->getGlobalColor();
		if (_Blinking)
		{
			const uint blinkDuration = 300;
			if (((_BlinkDT / 300) & 1) == 0)
			{
				CRGBA newCol = oldGlobalColor;
				clamp(newCol.R, 64, 192);
				clamp(newCol.G, 64, 192);
				clamp(newCol.B, 64, 192);
				newCol.R = ~newCol.R;
				newCol.G = ~newCol.G;
				newCol.B = ~newCol.B;
				if (abs(newCol.R - oldGlobalColor.R) < 64) newCol.R = 192;
				if (abs(newCol.G - oldGlobalColor.G) < 64) newCol.G = 192;
				if (abs(newCol.B - oldGlobalColor.B) < 64) newCol.B = 192;
				CWidgetManager::getInstance()->setGlobalColor(newCol);
				_BlinkState = true;
			}
			else
			{
				if (_BlinkState) // was previously on ?
				{
					if (_NumBlinks != 0) // if dont blink for ever
					{
						-- _NumBlinks;
						if (_NumBlinks == 0)
						{
							disableBlink();
						}
					}
				}
				_BlinkState = false;
			}
			_BlinkDT += std::min((uint) times.frameDiffMs, blinkDuration);
		}

		CGroupContainer *parentGC = NULL;
		if (getParent() && getParent()->getParent())
		{
			if (getParent()->getParent()->isGroupContainer())
			{
				parentGC = static_cast<CGroupContainer *>(getParent()->getParent());
			}
		}

		sint32 oldSciX, oldSciY, oldSciW, oldSciH;
		makeNewClip (oldSciX, oldSciY, oldSciW, oldSciH);

		// Draw the container
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		CRGBA col = CWidgetManager::getInstance()->getGlobalColor();

		bool bGrayed = isGrayed();
		if (bGrayed)
		{
			col.R = col.R / 2;
			col.G = col.G / 2;
			col.B = col.B / 2;
			CWidgetManager::getInstance()->setGlobalColor(col);
			oldGlobalColor.R = oldGlobalColor.R / 2;
			oldGlobalColor.G = oldGlobalColor.G / 2;
			oldGlobalColor.B = oldGlobalColor.B / 2;
		}

		if (_MovingInParentList)
		{
			// display half tone when moving in parent
			col.A >>= 1;
		}
		// if the father is a group container, do modulation too
		if (parentGC)
		{
			// _CurrentContainerAlpha = (uint8) (((uint16) parentGC->getCurrentContainerAlpha() * (uint16) _ContainerAlpha) >> 8);
			_CurrentContainerAlpha = parentGC->getCurrentContainerAlpha();
		}
		else
		{
			_CurrentContainerAlpha = _UseGlobalAlpha ? CWidgetManager::getInstance()->getGlobalContainerAlpha() : _ContainerAlpha;
		}
		// modulate by container alpha color
		col.A = (uint8) (((uint16) _CurrentContainerAlpha * (uint16) col.A) >> 8);
		// Modulate by parent rollover
		if (parentGC)
		{
			// _ICurrentRolloverAlpha = (uint8) (((uint16) parentGC->_ICurrentRolloverAlpha * (uint16) _ICurrentRolloverAlpha) >> 8);
			_ICurrentRolloverAlphaContent = parentGC->_ICurrentRolloverAlphaContent;
			_ICurrentRolloverAlphaContainer = parentGC->_ICurrentRolloverAlphaContainer;
		}
		else
		{
			uint8 rolloverFactorContent = _UseGlobalAlpha ? (255 - CWidgetManager::getInstance()->getGlobalRolloverFactorContent()) : _RolloverAlphaContent;
			_ICurrentRolloverAlphaContent = (uint8) (255 - rolloverFactorContent + rolloverFactorContent * _CurrentRolloverAlphaContent);
			uint8 rolloverFactorContainer = _UseGlobalAlpha ? (255 - CWidgetManager::getInstance()->getGlobalRolloverFactorContainer()) : _RolloverAlphaContainer;
			_ICurrentRolloverAlphaContainer = (uint8) (255 - rolloverFactorContainer + rolloverFactorContainer * _CurrentRolloverAlphaContainer);
		}
		// Modulate alpha by rollover alpha
		col.A = (uint8) (((uint16) _ICurrentRolloverAlphaContainer * (uint16) col.A) >> 8);
		//
		COptionsLayer *pLayer = getContainerOptions();
		// h is the size of what is on top of the child list
		sint32 x, y, w, h;

		bool bHasChild = (_List->getNbElement() > 0);
		h = (pLayer->H_T - pLayer->InsetT) + (((!_Opened) || (!bHasChild)) ? pLayer->H_B : pLayer->H_B_Open);

		if (_Opened)
		{
			if (_HeaderOpened != NULL)
				h += max (_HeaderOpened->getHReal(), pLayer->HeaderH);
			else
				h += pLayer->HeaderH;

			if (_Content != NULL)
				h += _Content->getHReal();

			h -= _ContentYOffset;
		}
		else
		{
			h = _HReal;
		}

		x = _XReal;
		y = _YReal+_HReal-h;
		w = _WReal;
		sint8 rl = _RenderLayer;
		if (_LayerSetup == 0)
		{
			// Top Left
			rVR.drawRotFlipBitmap (rl, x, y+h-pLayer->H_TL, pLayer->W_TL, pLayer->H_TL, 0, false, pLayer->TxId_TL, col);
			// Top
			if (pLayer->Tile_T == 0) // Tiling ?
				rVR.drawRotFlipBitmap (rl, x+pLayer->W_TL, y+h-pLayer->H_T, w-(pLayer->W_TL+pLayer->W_TR), pLayer->H_T, 0, false, pLayer->TxId_T, col);
			else
				rVR.drawRotFlipBitmapTiled (rl, x+pLayer->W_TL, y+h-pLayer->H_T, w-(pLayer->W_TL+pLayer->W_TR), pLayer->H_T, 0, false, pLayer->TxId_T, pLayer->Tile_T-1, col);
			// Top Right
			rVR.drawRotFlipBitmap (rl, x+w-pLayer->W_TR, y+h-pLayer->H_TR, pLayer->W_TR, pLayer->H_TR, 0, false, pLayer->TxId_TR, col);

			if ((!_Opened) || (!bHasChild))
			{ // Not opened
				// Left
				if (pLayer->Tile_L == 0) // Tiling ?
					rVR.drawRotFlipBitmap (rl, x, y+pLayer->H_BL, pLayer->W_L, h-(pLayer->H_BL+pLayer->H_TL), 0, false, pLayer->TxId_L, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x, y+pLayer->H_BL, pLayer->W_L, h-(pLayer->H_BL+pLayer->H_TL), 0, false, pLayer->TxId_L, pLayer->Tile_L-1, col);
				// Right
				if (pLayer->Tile_R == 0) // Tiling ?
					rVR.drawRotFlipBitmap (rl, x+w-pLayer->W_R, y+pLayer->H_BR, pLayer->W_R, h-(pLayer->H_TR+pLayer->H_BR), 0, false, pLayer->TxId_R, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x+w-pLayer->W_R, y+pLayer->H_BR, pLayer->W_R, h-(pLayer->H_TR+pLayer->H_BR), 0, false, pLayer->TxId_R, pLayer->Tile_R-1, col);
				// Bottom Left
				rVR.drawRotFlipBitmap (rl, x, y, pLayer->W_BL, pLayer->H_BL, 0, false, pLayer->TxId_BL, col);
				// Bottom
				if (pLayer->Tile_B == 0) // Tiling ?
					rVR.drawRotFlipBitmap (rl, x+pLayer->W_BL, y, w-(pLayer->W_BL+pLayer->W_BR), pLayer->H_B, 0, false, pLayer->TxId_B, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x+pLayer->W_BL, y, w-(pLayer->W_BL+pLayer->W_BR), pLayer->H_B, 0, false, pLayer->TxId_B, pLayer->Tile_B-1, col);
				// Bottom Right
				rVR.drawRotFlipBitmap (rl, x+w-pLayer->W_BR, y, pLayer->W_BR, pLayer->H_BR, 0, false, pLayer->TxId_BR, col);
				// Content
				if (pLayer->Tile_Blank == 0) // Tiling ?
					rVR.drawRotFlipBitmap (rl, x+pLayer->W_L, y+pLayer->H_B, w-(pLayer->W_R+pLayer->W_L), h-(pLayer->H_B+pLayer->H_T), 0, false, pLayer->TxId_Blank, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x+pLayer->W_L, y+pLayer->H_B, w-(pLayer->W_R+pLayer->W_L), h-(pLayer->H_B+pLayer->H_T), 0, false, pLayer->TxId_Blank, pLayer->Tile_Blank-1, col);
			}
			else
			{ // Opened
				// Left
				if (pLayer->Tile_L == 0) // Tiling ?
					rVR.drawRotFlipBitmap (rl, x, y+pLayer->H_BL_Open, pLayer->W_L, h-(pLayer->H_BL_Open+pLayer->H_TL), 0, false, pLayer->TxId_L, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x, y+pLayer->H_BL_Open, pLayer->W_L, h-(pLayer->H_BL_Open+pLayer->H_TL), 0, false, pLayer->TxId_L, pLayer->Tile_L-1, col);
				// Right
				if (pLayer->Tile_R == 0) // Tiling ?
					rVR.drawRotFlipBitmap (rl, x+w-pLayer->W_R, y+pLayer->H_BR_Open, pLayer->W_R, h-(pLayer->H_TR+pLayer->H_BR_Open), 0, false, pLayer->TxId_R, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x+w-pLayer->W_R, y+pLayer->H_BR_Open, pLayer->W_R, h-(pLayer->H_TR+pLayer->H_BR_Open), 0, false, pLayer->TxId_R, pLayer->Tile_R-1, col);
				// Bottom Left
				rVR.drawRotFlipBitmap (rl, x, y, pLayer->W_BL_Open, pLayer->H_BL_Open, 0, false, pLayer->TxId_BL_Open, col);
				// Bottom
				if (pLayer->Tile_B_Open == 0) // Tiling ?
					rVR.drawRotFlipBitmap (rl, x+pLayer->W_BL_Open, y, w-(pLayer->W_BL_Open+pLayer->W_BR_Open), pLayer->H_B_Open, 0, false, pLayer->TxId_B_Open, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x+pLayer->W_BL_Open, y, w-(pLayer->W_BL_Open+pLayer->W_BR_Open), pLayer->H_B_Open, 0, false, pLayer->TxId_B_Open, pLayer->Tile_B_Open-1, col);
				// Bottom Right
				rVR.drawRotFlipBitmap (rl, x+w-pLayer->W_BR_Open, y, pLayer->W_BR_Open, pLayer->H_BR_Open, 0, false, pLayer->TxId_BR_Open, col);
				// Content
				if (pLayer->Tile_Blank == 0) // Tiling ?
					rVR.drawRotFlipBitmap (rl, x+pLayer->W_L, y+pLayer->H_B_Open, w-(pLayer->W_R+pLayer->W_L), h-(pLayer->H_B_Open+pLayer->H_T), 0, false, pLayer->TxId_Blank, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x+pLayer->W_L, y+pLayer->H_B_Open, w-(pLayer->W_R+pLayer->W_L), h-(pLayer->H_B_Open+pLayer->H_T), 0, false, pLayer->TxId_Blank, pLayer->Tile_Blank-1, col);
				// ScrollBar Placement
				if (pLayer->Tile_M_Open == 0)  // Tiling ?
					rVR.drawRotFlipBitmap (rl, x, _YReal+pLayer->H_EL_Open, pLayer->W_M_Open, _HReal-h-pLayer->H_EL_Open, 0, false, pLayer->TxId_M_Open, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x, _YReal+pLayer->H_EL_Open, pLayer->W_M_Open, _HReal-h-pLayer->H_EL_Open, 0, false, pLayer->TxId_M_Open, pLayer->Tile_M_Open-1, col);
				// Ending Left
				rVR.drawRotFlipBitmap (rl, x, _YReal, pLayer->W_EL_Open, pLayer->H_EL_Open, 0, false, pLayer->TxId_EL_Open, col);
				// Ending Middle
				if (pLayer->Tile_EM_Open == 0) // Tiling ?
					rVR.drawRotFlipBitmap (rl, x+pLayer->W_EL_Open, _YReal, w-(pLayer->W_EL_Open+pLayer->W_ER_Open), pLayer->H_EM_Open, 0, false, pLayer->TxId_EM_Open, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x+pLayer->W_EL_Open, _YReal, w-(pLayer->W_EL_Open+pLayer->W_ER_Open), pLayer->H_EM_Open, 0, false, pLayer->TxId_EM_Open, pLayer->Tile_EM_Open-1, col);
				// Ending Right
				rVR.drawRotFlipBitmap (rl, x+w-pLayer->W_ER_Open, _YReal, pLayer->W_ER_Open, pLayer->H_ER_Open, 0, false, pLayer->TxId_ER_Open, col);
			}
		}
		else // Version for layer 1,2,3
		{
			// Top Left
			rVR.drawRotFlipBitmap (rl, x, y+h-pLayer->H_TL, pLayer->W_TL, pLayer->H_TL, 0, false, pLayer->TxId_TL, col);
			// Top
			if (pLayer->Tile_T == 0) // Tiling ?
				rVR.drawRotFlipBitmap (rl, x+pLayer->W_TL, y+h-pLayer->H_T, w-(pLayer->W_TL+pLayer->W_TR), pLayer->H_T, 0, false, pLayer->TxId_T, col);
			else
				rVR.drawRotFlipBitmapTiled (rl, x+pLayer->W_TL, y+h-pLayer->H_T, w-(pLayer->W_TL+pLayer->W_TR), pLayer->H_T, 0, false, pLayer->TxId_T, pLayer->Tile_T-1, col);
			// Top Right
			rVR.drawRotFlipBitmap (rl, x+w-pLayer->W_TR, y+h-pLayer->H_TR, pLayer->W_TR, pLayer->H_TR, 0, false, pLayer->TxId_TR, col);

			if ((!_Opened) || (!bHasChild))
			{
				// Left
				if (pLayer->Tile_L == 0) // Tiling ?
					rVR.drawRotFlipBitmap (rl, x, y+pLayer->H_BL, pLayer->W_L, h-(pLayer->H_BL+pLayer->H_TL), 0, false, pLayer->TxId_L, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x, y+pLayer->H_BL, pLayer->W_L, h-(pLayer->H_BL+pLayer->H_TL), 0, false, pLayer->TxId_L, pLayer->Tile_L-1, col);
				// Right
				if (pLayer->Tile_R == 0) // Tiling ?
					rVR.drawRotFlipBitmap (rl, x+w-pLayer->W_R, y+pLayer->H_BR, pLayer->W_R, h-(pLayer->H_BL+pLayer->H_TL), 0, false, pLayer->TxId_R, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x+w-pLayer->W_R, y+pLayer->H_BR, pLayer->W_R, h-(pLayer->H_BL+pLayer->H_TL), 0, false, pLayer->TxId_R, pLayer->Tile_R-1, col);
				// Bottom Left
				rVR.drawRotFlipBitmap (rl, x, y, pLayer->W_BL, pLayer->H_BL, 0, false, pLayer->TxId_BL, col);
				// Bottom
				if (pLayer->Tile_B == 0) // Tiling ?
					rVR.drawRotFlipBitmap (rl, x+pLayer->W_BL, y, w-(pLayer->W_BL+pLayer->W_BR), pLayer->H_B, 0, false, pLayer->TxId_B, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x+pLayer->W_BL, y, w-(pLayer->W_BL+pLayer->W_BR), pLayer->H_B, 0, false, pLayer->TxId_B, pLayer->Tile_B-1, col);
				// Bottom Right
				rVR.drawRotFlipBitmap (rl, x+w-pLayer->W_BR, y, pLayer->W_BR, pLayer->H_BR, 0, false, pLayer->TxId_BR, col);
				// Content
				if (pLayer->Tile_Blank == 0) // Tiling ?
					rVR.drawRotFlipBitmap (rl, x+pLayer->W_L, y+pLayer->H_B, w-(pLayer->W_L+pLayer->W_R), h-(pLayer->H_B+pLayer->H_T), 0, false, pLayer->TxId_Blank, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x+pLayer->W_L, y+pLayer->H_B, w-(pLayer->W_L+pLayer->W_R), h-(pLayer->H_B+pLayer->H_T), 0, false, pLayer->TxId_Blank, pLayer->Tile_Blank-1, col);
			}
			else
			{
				// Left
				if (pLayer->Tile_L == 0) // Tiling ?
					rVR.drawRotFlipBitmap (rl, x, y+pLayer->H_BL_Open, pLayer->W_L, h-(pLayer->H_BL_Open+pLayer->H_TL), 0, false, pLayer->TxId_L, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x, y+pLayer->H_BL_Open, pLayer->W_L, h-(pLayer->H_BL_Open+pLayer->H_TL), 0, false, pLayer->TxId_L, pLayer->Tile_L-1, col);
				// Right
				if (pLayer->Tile_R == 0) // Tiling ?
					rVR.drawRotFlipBitmap (rl, x+w-pLayer->W_R, y+pLayer->H_BR_Open, pLayer->W_R, h-(pLayer->H_TR+pLayer->H_BR_Open), 0, false, pLayer->TxId_R, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x+w-pLayer->W_R, y+pLayer->H_BR_Open, pLayer->W_R, h-(pLayer->H_TR+pLayer->H_BR_Open), 0, false, pLayer->TxId_R, pLayer->Tile_R-1, col);
				// Bottom Left
				rVR.drawRotFlipBitmap (rl, x, y, pLayer->W_BL_Open, pLayer->H_BL_Open, 0, false, pLayer->TxId_BL_Open, col);
				// Bottom
				if (pLayer->Tile_B_Open == 0) // Tiling ?
					rVR.drawRotFlipBitmap (rl, x+pLayer->W_BL_Open, y, w-(pLayer->W_BL_Open+pLayer->W_BR_Open), pLayer->H_B_Open, 0, false, pLayer->TxId_B_Open, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x+pLayer->W_BL_Open, y, w-(pLayer->W_BL_Open+pLayer->W_BR_Open), pLayer->H_B_Open, 0, false, pLayer->TxId_B_Open, pLayer->Tile_B_Open-1, col);
				// Bottom Right
				rVR.drawRotFlipBitmap (rl, x+w-pLayer->W_BR_Open, y, pLayer->W_BR_Open, pLayer->H_BR_Open, 0, false, pLayer->TxId_BR_Open, col);
				// Content
				if (pLayer->Tile_Blank == 0)
					rVR.drawRotFlipBitmap (rl, x+pLayer->W_L, y+pLayer->H_B_Open, w-(pLayer->W_L+pLayer->W_R), h-(pLayer->H_B_Open+pLayer->H_T), 0, false, pLayer->TxId_Blank, col);
				else
					rVR.drawRotFlipBitmapTiled (rl, x+pLayer->W_L, y+pLayer->H_B_Open, w-(pLayer->W_L+pLayer->W_R), h-(pLayer->H_B_Open+pLayer->H_T), 0, false, pLayer->TxId_Blank, pLayer->Tile_Blank-1, col);
			}
		}


		if (_Blinking)
		{
			CWidgetManager::getInstance()->setGlobalColor(oldGlobalColor);
		}

		// Top window : title is highlighted
		if (_LayerSetup == 0)
		{
			CRGBA c = CRGBA(255,255,255,255);
			// Display the header in white if we are the last clicked window
			if (CWidgetManager::getInstance()->getTopWindow(CWidgetManager::getInstance()->getLastTopWindowPriority()) != this)
			{
				if (_HeaderColor.getNodePtr() != NULL)
					c = _HeaderColor.getRGBA();
				if (bGrayed)
				{
					c.R = c.R / 2;
					c.G = c.G / 2;
					c.B = c.B / 2;
				}
				c.A = 255;
			}
			if (_TitleClosed != NULL) _TitleClosed->setColor(c);
			if (_TitleOpened != NULL) _TitleOpened->setColor(c);
			if (_ViewOpenState != NULL) _ViewOpenState->setColor(c);
			if (_RightButton != NULL)
			{
				_RightButton->setColor(c);
				_RightButton->setColorPushed(c);
				_RightButton->setColorOver(c);
			}
			if (_HelpButton != NULL)
			{
				_HelpButton->setColor(c);
				_HelpButton->setColorPushed(c);
				_HelpButton->setColorOver(c);
			}
		}

		// Render inside window

		uint8 oldAlphaContent = CWidgetManager::getInstance()->getContentAlpha();
		uint8 oldAlphaContainer = _CurrentContainerAlpha;
		if (parentGC)
		{
			// _CurrentContentAlpha = (uint8) (((uint16) _ContentAlpha * (uint16) parentGC->getCurrentContentAlpha()) >> 8);
			_CurrentContentAlpha = parentGC->getCurrentContentAlpha();
			_CurrentContainerAlpha = parentGC->getCurrentContainerAlpha();
		}
		else
		{
			_CurrentContentAlpha = _UseGlobalAlpha ? CWidgetManager::getInstance()->getGlobalContentAlpha() : _ContentAlpha;
			_CurrentContainerAlpha = _UseGlobalAlpha ? CWidgetManager::getInstance()->getGlobalContainerAlpha() : _ContainerAlpha;
		}
		// set content alpha multiplied by rollover alpha
		CWidgetManager::getInstance()->setContentAlpha((uint8) (((uint16) _CurrentContentAlpha * (uint16) _ICurrentRolloverAlphaContent) >> 8));
		// set content alpha multiplied by rollover alpha
		_CurrentContainerAlpha = (uint8) (((uint16) _CurrentContainerAlpha * (uint16) _ICurrentRolloverAlphaContainer) >> 8);

		// Display the color title bar (if the header is active)
		if (_LayerSetup == 0)
		{
			if (_HeaderActive)
			{
				CRGBA c(255,255,255,255);
				if (_HeaderColor.getNodePtr() != NULL)
					c = _HeaderColor.getRGBA();
				if (bGrayed)
				{
					c.R = c.R / 2;
					c.G = c.G / 2;
					c.B = c.B / 2;
				}
				c.A = (uint8) (((uint16) _CurrentContentAlpha * (uint16) _ICurrentRolloverAlphaContent) >> 8);
				// Left
 				rVR.drawRotFlipBitmap (rl, x, y+h-pLayer->H_L_Header, pLayer->W_L_Header, pLayer->H_L_Header, 0, false, pLayer->TxId_L_Header, c);
				// Middle
				if (pLayer->Tile_M_Header == 0) // Tiling ?
					rVR.drawRotFlipBitmap (rl, x+pLayer->W_L_Header, y+h-pLayer->H_M_Header, w-(pLayer->W_L_Header+pLayer->W_R_Header), pLayer->H_M_Header, 0, false, pLayer->TxId_M_Header, c);
				else
					rVR.drawRotFlipBitmapTiled (rl, x+pLayer->W_L_Header, y+h-pLayer->H_M_Header, w-(pLayer->W_L_Header+pLayer->W_R_Header), pLayer->H_M_Header, 0, false, pLayer->TxId_M_Header, pLayer->Tile_M_Header-1, c);
				// Right
				rVR.drawRotFlipBitmap (rl, x+w-pLayer->W_R_Header, y+h-pLayer->H_R_Header, pLayer->W_R_Header, pLayer->H_R_Header, 0, false, pLayer->TxId_R_Header, c);
			}
		}


		CInterfaceGroup::draw();


		if (_LayerSetup == 0)
		if ((_HighLighted) || (_CurrentContainerAlpha <= 128))
		{
			uint8 nInverted = (128-_CurrentContainerAlpha)/2;
			if (!_HighLighted)
				col.A = nInverted;
			else
				col.A = max(_HighLightedAlpha, nInverted);
			// corners
			rVR.drawRotFlipBitmap (_RenderLayer, x, y + h - pLayer->H_T_HighLight, pLayer->W_TL_HighLight, pLayer->H_TL_HighLight, 0, false, pLayer->TxId_TL_HighLight, col);
			rVR.drawRotFlipBitmap (_RenderLayer, x + _WReal - pLayer->W_TR_HighLight, y + h - pLayer->H_T_HighLight, pLayer->W_TR_HighLight, pLayer->H_TR_HighLight, 0, false, pLayer->TxId_TR_HighLight, col);
			rVR.drawRotFlipBitmap (_RenderLayer, x, _YReal, pLayer->W_BL_HighLight, pLayer->H_BL_HighLight, 0, false, pLayer->TxId_BL_HighLight, col);
			rVR.drawRotFlipBitmap (_RenderLayer, x + _WReal - pLayer->W_BR_HighLight, _YReal, pLayer->W_BR_HighLight, pLayer->H_BR_HighLight, 0, false, pLayer->TxId_BR_HighLight, col);
			// border
			rVR.drawRotFlipBitmap (_RenderLayer, x + pLayer->W_TL_HighLight, y + h - pLayer->H_T_HighLight, _WReal - pLayer->W_TL_HighLight - pLayer->W_TR_HighLight, pLayer->H_T_HighLight, 0, false, pLayer->TxId_T_HighLight, col);
			rVR.drawRotFlipBitmap (_RenderLayer, x + pLayer->W_BL_HighLight, _YReal, _WReal - pLayer->W_BL_HighLight - pLayer->W_BR_HighLight, pLayer->H_B_HighLight, 0, false, pLayer->TxId_B_HighLight, col);
			rVR.drawRotFlipBitmap (_RenderLayer, x, _YReal + pLayer->H_B_HighLight, pLayer->W_L_HighLight, _HReal - pLayer->H_T_HighLight - pLayer->H_B_HighLight, 0, false, pLayer->TxId_L_HighLight, col);
			rVR.drawRotFlipBitmap (_RenderLayer, x + _WReal - pLayer->W_R_HighLight, _YReal + pLayer->H_B_HighLight, pLayer->W_R_HighLight, _HReal - pLayer->H_T_HighLight - pLayer->H_B_HighLight, 0, false, pLayer->TxId_R_HighLight, col);
		}


		CWidgetManager::getInstance()->setContentAlpha(oldAlphaContent);
		_CurrentContainerAlpha = oldAlphaContainer;


		// manage rollover
		CViewPointerBase *mousePointer = CWidgetManager::getInstance()->getPointer();
 		if (mousePointer)
		{
			bool dontFade = false;
	//		bool alphaUp = false;
			// should not applied if the container is being resized
			if (CWidgetManager::getInstance()->getCapturePointerLeft() != NULL)
			{
				CInterfaceGroup *ig = CWidgetManager::getInstance()->getCapturePointerLeft()->getParent();
				while (ig)
				{
					if (ig == this)
					{
						dontFade = true;
						break;
					}
					ig = ig->getParent();
				}
			}

			if (hasKeyboardFocus()) dontFade =true;

			bool isOver = false;

			if (CWidgetManager::getInstance()->getCapturePointerLeft() == NULL)
			if (isIn(mousePointer->getX(), mousePointer->getY()))
			{
				CInterfaceGroup *ig = CWidgetManager::getInstance()->getCurrentWindowUnder();
				while (ig)
				{
					if (ig == this)
					{
						isOver = true;
						break;
					}
					ig = ig->getParent();
				}
			}
			if (dontFade || isOver)
			{
				_CurrentRolloverAlphaContent += (float) (speed * times.frameDiffMs);
				_CurrentRolloverAlphaContent = std::min(1.f, _CurrentRolloverAlphaContent);

				_CurrentRolloverAlphaContainer += (float) (speed * times.frameDiffMs);
				_CurrentRolloverAlphaContainer = std::min(1.f, _CurrentRolloverAlphaContainer);
			}
			else
			{
				_CurrentRolloverAlphaContent -= (float) (speed * times.frameDiffMs);
				_CurrentRolloverAlphaContent = std::max(0.f, _CurrentRolloverAlphaContent);

				_CurrentRolloverAlphaContainer -= (float) (speed * times.frameDiffMs);
				_CurrentRolloverAlphaContainer = std::max(0.f, _CurrentRolloverAlphaContainer);
			}
		}

		if (bGrayed)
		{
			CWidgetManager::getInstance()->setGlobalColor(oldGColForGrayed);
		}


		// Restore the old clip window
		restoreClip (oldSciX, oldSciY, oldSciW, oldSciH);
	}

	// ***************************************************************************
	void CGroupContainer::clearViews()
	{
		CInterfaceGroup::clearViews();
	}

	// ***************************************************************************
	bool CGroupContainer::handleEvent (const NLGUI::CEventDescriptor& event)
	{
		if (!_Active)
			return false;

		if (_MovingInParentList)
		{
			return true;
		}

		if (!checkIfModal(event))
			return false;

		if (!CInterfaceGroup::handleEvent(event))
		{
			if (event.getType() == NLGUI::CEventDescriptor::mouse)
			{
				const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;
				// MouseWheel mgt
				if ((_LayerSetup == 0) && (isIn(eventDesc.getX(), eventDesc.getY())))
				{
					if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousewheel)
					{
						if (_ScrollBar != NULL)
							_ScrollBar->moveTrackY (eventDesc.getWheel()*12);
						return true;
					}
				}
			}
			return false;
		}

		return true;
	}

	// ***************************************************************************
	void CGroupContainer::open()
	{
		disableBlink();
		_Opened = true;
		_OpenAtStart = true;
		if (_TitleOpened != NULL) _TitleOpened->setActive(true);
		if (_TitleClosed != NULL) _TitleClosed->setActive(false);

		if ((_Openable) && (_LayerSetup >= 0))
		{
			COptionsLayer *pLayer = getContainerOptions();
			if (_ViewOpenState != NULL) _ViewOpenState->setTexture(pLayer->getValStr ("open_state_tx_opened"));
		}

		if (_List != NULL) _List->setActive(true);

		if (_ScrollBar != NULL)		_ScrollBar->setActive(true);
		if (_Content != NULL)		_Content->setActive(true);
		if (_HeaderClosed != NULL)	_HeaderClosed->setActive(false);
		if (_HeaderOpened != NULL)	_HeaderOpened->setActive(true);

		invalidateCoords();

		// call action handler if any
		if (_AHOnOpen != NULL)
		{
			CAHManager::getInstance()->runActionHandler(_AHOnOpen, this, _AHOnOpenParams);
		}

	}

	// ***************************************************************************
	void CGroupContainer::close()
	{
		_Opened = false;
		if (_TitleOpened != NULL) _TitleOpened->setActive(false);
		if (_TitleClosed != NULL) _TitleClosed->setActive(true);
		if ((_Openable) && (_LayerSetup >= 0))
		{
			COptionsLayer *pLayer = getContainerOptions();
			if (_ViewOpenState != NULL) _ViewOpenState->setTexture(pLayer->getValStr ("open_state_tx_closed"));
		}
		if (_List != NULL) _List->setActive(false);

		if (_ScrollBar != NULL)		_ScrollBar->setActive(false);
		if (_Content != NULL)		_Content->setActive(false);
		if (_HeaderClosed != NULL)	_HeaderClosed->setActive(true);
		if (_HeaderOpened != NULL)	_HeaderOpened->setActive(false);
		invalidateCoords();

		// call action handler if any
		if (_AHOnClose != NULL)
		{
			CAHManager::getInstance()->runActionHandler(_AHOnClose, this, _AHOnCloseParams);
		}
	}

	// ***************************************************************************
	void CGroupContainer::attachContainer (CGroupContainer *pIC, sint insertionOrder /* = -1 */)
	{
		if (_List == NULL)
		{
			_List = new CGroupList(CViewBase::TCtorParam());
			_List->setId (_Id+":list");
			_List->setParent (this);
		}

		// Remove from the list if already inserted
		_List->delChild (pIC, true);
		if (insertionOrder == -1)
		{
			_List->addChild (pIC, false); // Do not delete it on remove !
		}
		else
		{
			uint k = 0;
			for(k = 0; k < (uint) _List->getNbElement(); ++k)
			{
				if (_List->getOrder(k) > (uint) insertionOrder) break;
			}
			_List->addChildAtIndex(pIC, k, false);
			_List->setOrder(k, insertionOrder);
		}

		// Create MaxH Resizer if not already created
		createResizerMaxH();
	}


	// ***************************************************************************
	bool CGroupContainer::attachContainerAtIndex(CGroupContainer *pIC, uint index)
	{
		if (_List == NULL)
		{
			_List = new CGroupList(CViewBase::TCtorParam());
			_List->setId (_Id+":list");
			_List->setParent (this);
		}
		if (index > (uint) _List->getNbElement())
		{
			nlwarning("Bad index");
			return false;
		}
		uint eltOrder;
		if (index == (uint) _List->getNbElement())
		{
			if (_List->getNbElement() == 0)
			{
				eltOrder = 0;
			}
			else
			{
				eltOrder = _List->getOrder(index - 1);
			}
		}
		else
		{
			eltOrder = _List->getOrder(index);
		}
		uint k;
		for(k = index; k < (uint) _List->getNbElement(); ++k)
		{
			_List->setOrder(k, _List->getOrder(k) + 1);
		}
		// change insertion order of poped containers
		for(k = 0; k < _PopedCont.size(); ++k)
		{
			if (_PopedCont[k]->_InsertionOrder >= eltOrder)
			{
				++ _PopedCont[k]->_InsertionOrder;
			}
		}
		attachContainer(pIC, eltOrder);
		return true;
	}


	// ***************************************************************************
	void CGroupContainer::detachContainer (CGroupContainer *pIC)
	{
		if (!pIC) return;
		nlassert(_List != NULL);
		nlassert(pIC->getProprietaryContainer() == this); // should be a son of that container
		nlassert(!pIC->isPopuped()); // if the container is poped, should pop it in before detaching it!
		_List->delChild (pIC);
		_List->invalidateCoords(2);

		// Remove MaxH Resizer if exist
		if (_List->getNumChildren() == 0)
		{
			removeResizerMaxH();
		}

	}

	// ***************************************************************************
	void CGroupContainer::removeAllContainers()
	{
		if (!_List) return;
		_List->deleteAllChildren();
	}

	// ***************************************************************************
	void CGroupContainer::setMovable(bool b)
	{
		if (_Movable != b)
		{
			_Movable = b;
			if (_LayerSetup != -1)
			{
				updateMover();
			}
		}
	}

	// Make from layer
	// ***************************************************************************
	void CGroupContainer::setup()
	{
		sint32 nNewLayer = getLayer();
		if (_LayerSetup == nNewLayer)
			return;

		_LayerSetup = nNewLayer;

		if ((_LayerSetup == 0) && _Popable)
		{
			_Poped = true;
		}
		else
		{
			_Poped = false;
		}


		COptionsLayer *pLayer = getContainerOptions(_LayerSetup);

		if (_LayerSetup == 0)
		{
			setParentPosRef(Hotspot_BL);
			setPosRef(Hotspot_TL);
		}
		else
		{
			setParentPosRef(Hotspot_TL);
			setPosRef(Hotspot_TL);
		}

		// At start, consider all closed.

		// Title when the container is opened
		updateTitle();

		// Opened state view (tells the user if the container is openable (if the view appears) and if its opened/closed
		updateViewOpenState();

		// Multi usage button
		updateRightButton();

		// Help button
		updateHelpButton();

		// if the window is popable,

		if (_List == NULL)
			_List = new CGroupList(CViewBase::TCtorParam());
		_List->setId(_Id+":list");
		_List->setParent (this);
		_List->setParentPos (this);
		_List->setParentPosRef (Hotspot_TL);
		_List->setPosRef (Hotspot_TL);
		_List->setActive (_Opened);

		if (_LayerSetup == 0)
		{
			_ScrollBar = new CCtrlScroll(CViewBase::TCtorParam());
			_ScrollBar->setId (_Id+":sb");
			_ScrollBar->setParent (this);
			_ScrollBar->setParentPos (this);
			_ScrollBar->setParentPosRef (Hotspot_TL);
			_ScrollBar->setPosRef (Hotspot_TL);
			_ScrollBar->setTarget(_List);
			_ScrollBar->setW (pLayer->Scrollbar_W); // TODO read this from somewhere
			_ScrollBar->setAlign (3); // Top
			_ScrollBar->setTextureBottomOrLeft	(pLayer->TxId_B_Scrollbar);
			_ScrollBar->setTextureMiddle		(pLayer->TxId_M_Scrollbar);
			_ScrollBar->setTextureTopOrRight	(pLayer->TxId_T_Scrollbar);
			_ScrollBar->setTextureMiddleTile	((uint8)pLayer->Tile_M_Scrollbar);
			_ScrollBar->setActive (false);
		}
		else
		{
			_ScrollBar = NULL;
		}


		// del all previous resizers
		uint k;
		for(k = 0; k < NumResizers; ++k)
		{
			if (_Resizer[k])
			{
				delCtrl (toString("rz%d", (int) k));
				_Resizer[k] = NULL;
			}
		}

		// Create Mover
		updateMover();

		// Remove previous controls / views

		delGroup ("list", true);
		delCtrl ("sb");

		COptionsContainerMove *options = getMoveOptions();

		// create resizer?
		if ((_LayerSetup == 0) && (_EnabledResizer))
		{
			if (options)
			{
				sint	yOffset;
				// if no specific top Size specified
				if(_ResizerTopSize<0)
					yOffset= -options->ResizerSize;
				else
					yOffset= -_ResizerTopSize;
				// create the resizers
				createResizer(0, Hotspot_TL, Hotspot_TM, options->ResizerSize, 0, false);
				createResizer(1, Hotspot_TR, Hotspot_TR, 0, 0, false);
				createResizer(2, Hotspot_TR, Hotspot_MR, 0, yOffset, false);
				createResizer(3, Hotspot_BR, Hotspot_BR, 0, 0, false);
				createResizer(4, Hotspot_BR, Hotspot_BM, -options->ResizerSize, 0, false);
				createResizer(5, Hotspot_BL, Hotspot_BL, 0, 0, false);
				createResizer(6, Hotspot_TL, Hotspot_ML, 0, yOffset, false);
				createResizer(7, Hotspot_TL, Hotspot_TL, 0, 0, false);
			}
		}

		if ((_LayerSetup == 0) && (options))
		{
			createResizerMaxH();
		}

		addGroup (_List);
		if (_ScrollBar != NULL)		addCtrl (_ScrollBar);

		// Link with script

		_Content = getGroup ("content");
		if (_Content != NULL)
		{
			// Content must be (TL TL), (TR TR) or (TM TM)
			_Content->setPosRef( (THotSpot)((_Content->getPosRef() & (Hotspot_xL|Hotspot_xM|Hotspot_xR)) | Hotspot_Tx) );
			_Content->setParentPosRef(_Content->getPosRef());
			_Content->setActive (false);
		}

		_HeaderOpened = getGroup ("header_opened");
		if (_HeaderOpened != NULL)
		{
			// Header opened must be (TL TL), (TR TR) or (TM TM)
			_HeaderOpened->setPosRef( (THotSpot)((_HeaderOpened->getPosRef() & (Hotspot_xL|Hotspot_xM|Hotspot_xR)) | Hotspot_Tx) );
			_HeaderOpened->setParentPosRef(_HeaderOpened->getPosRef());
			_HeaderOpened->setActive (_Opened);
		}

		_HeaderClosed = getGroup ("header_closed");
		if (_HeaderClosed != NULL)
		{
			// Header closed must be (TL TL), (TR TR) or (TM TM)
			_HeaderClosed->setPosRef( (THotSpot)((_HeaderClosed->getPosRef() & (Hotspot_xL|Hotspot_xM|Hotspot_xR)) | Hotspot_Tx) );
			_HeaderClosed->setParentPosRef(_HeaderClosed->getPosRef());
			_HeaderClosed->setActive (!_Opened);
		}

		_List->setActive(_Opened);
		if (_ScrollBar != NULL)	_ScrollBar->setActive(_Opened);
		if (_Content != NULL)	_Content->setActive(_Opened);

		if (!_ModalParentNames.empty())
		{
			// reassign the first setup time
			setModalParentList(_ModalParentNames);
		}

		invalidateCoords ();
	}


	// ***************************************************************************
	// Update right button depending on pop, popable, layer and locked
	void CGroupContainer::updateRightButton()
	{
		if ((_EnabledRightButton) && (!((_LayerSetup>0)&&(!_Popable))))
		{
			COptionsLayer *pLayer = getContainerOptions();

			// Create right button
			if (_RightButton == NULL)
			{
				_RightButton = new CCtrlButton(CViewBase::TCtorParam());
				_RightButton->setId(_Id+":rightbut");
				_RightButton->setType(CCtrlBaseButton::PushButton);
				_RightButton->setParent (this);
				_RightButton->setParentPos (this);
				_RightButton->setParentPosRef (Hotspot_TR);
				_RightButton->setPosRef (Hotspot_TR);
				_RightButton->setModulateGlobalColorAll (getModulateGlobalColor());
				_RightButton->setModulateGlobalColorOver (true);
				addCtrl (_RightButton);
			}
			_RightButton->setX(pLayer->getValSInt32 ("right_button_offset_x"));
			_RightButton->setY(pLayer->getValSInt32 ("right_button_offset_y"));

			if (_LayerSetup == 0)
			{
				if (_Locked)
				{
					_RightButton->setTexture (pLayer->getValStr ("right_button_tx_locked"));
					_RightButton->setTexturePushed (pLayer->getValStr ("right_button_tx_locked"));
					_RightButton->setTextureOver ("");
					_RightButton->setActionOnLeftClick ("");
					_RightButton->setDefaultContextHelp(string(""));
				}
				else
				{
					// If the container is normally a layer>0 and is poped ? popin button
					if (_OldFatherContainer != NULL)
					{
						_RightButton->setTexture (pLayer->getValStr ("right_button_tx_popin"));
						_RightButton->setTexturePushed (pLayer->getValStr ("right_button_tx_popin"));
						_RightButton->setTextureOver (pLayer->getValStr ("right_button_tx_over"));
						_RightButton->setActionOnLeftClick ("ic_popin");
						_RightButton->setDefaultContextHelp(CI18N::get("uiPopinWindow"));
					}
					else // else the container can be deactivated ? deactivate button
					{
						_RightButton->setTexture (pLayer->getValStr ("right_button_tx_deactive"));
						_RightButton->setTexturePushed (pLayer->getValStr ("right_button_tx_deactive"));
						_RightButton->setTextureOver (pLayer->getValStr ("right_button_tx_over"));
						_RightButton->setActionOnLeftClick ("ic_deactive");
						_RightButton->setDefaultContextHelp(CI18N::get("uiClose"));
					}
				}
			}
			else
			{
				// If the container can be a popup ? popup button
				if (_Popable)
				{
					_RightButton->setTexture (pLayer->getValStr ("right_button_tx_popup"));
					_RightButton->setTexturePushed (pLayer->getValStr ("right_button_tx_popup"));
					_RightButton->setTextureOver (pLayer->getValStr ("right_button_tx_over"));
					_RightButton->setActionOnLeftClick ("ic_popup");
					_RightButton->setDefaultContextHelp(CI18N::get("uiPopupWindow"));
				}
				_RightButton->setActive(!_Locked);
			}
		}
		else
		{
			// Delete right button
			delCtrl ("rightbut");
			_RightButton = NULL;
		}
	}


	// ***************************************************************************
	void CGroupContainer::updateHelpButton()
	{
		// enable the help button only if has some help page
		if ( _EnabledHelpButton && !_HelpPage.empty() )
		{
			COptionsLayer *pLayer = getContainerOptions();

			// Create Help button
			if (_HelpButton == NULL)
			{
				_HelpButton = new CCtrlButton(CViewBase::TCtorParam());
				_HelpButton->setId(_Id+":helpbut");
				_HelpButton->setType(CCtrlBaseButton::PushButton);
				_HelpButton->setParent (this);
				_HelpButton->setParentPos (this);
				_HelpButton->setParentPosRef (Hotspot_TR);
				_HelpButton->setPosRef (Hotspot_TR);
				_HelpButton->setModulateGlobalColorAll (getModulateGlobalColor());
				_HelpButton->setModulateGlobalColorOver (true);
				addCtrl (_HelpButton);

				_HelpButton->setX(pLayer->getValSInt32 ("help_button_offset_x"));
				_HelpButton->setY(pLayer->getValSInt32 ("help_button_offset_y"));

				_HelpButton->setTexture (pLayer->getValStr ("help_button_tx"));
				_HelpButton->setTexturePushed (pLayer->getValStr ("help_button_tx"));
				_HelpButton->setTextureOver (pLayer->getValStr ("help_button_tx_over"));
				_HelpButton->setActionOnLeftClick ("ic_help");
				_HelpButton->setDefaultContextHelp(CI18N::get("uiHelp"));
			}

			// if not layer 0
			if (_LayerSetup > 0)
			{
				// if locked, the right button is not displayed => take its pos instead
				if(_Locked)
				{
					_HelpButton->setX(pLayer->getValSInt32 ("help_button_offset_x"));
					_HelpButton->setY(pLayer->getValSInt32 ("help_button_offset_y"));
				}
				else
				{
					_HelpButton->setX(pLayer->getValSInt32 ("right_button_offset_x"));
					_HelpButton->setY(pLayer->getValSInt32 ("right_button_offset_y"));
				}
			}
		}
		else
		{
			// Delete help button
			delCtrl ("helpbut");
			_HelpButton = NULL;
		}
	}


	// ***************************************************************************
	void CGroupContainer::updateMover()
	{
		if (_Movable || _Popable || _Openable)
		{
			COptionsContainerMove *options = getMoveOptions();
			_Mover = new CCtrlMover(CViewText::TCtorParam(), _Movable || _Popable || _MovableInParentList, _Openable);
			_Mover->setId (_Id+":mover");
			_Mover->setParent (this);
			_Mover->setParentPos (this);
			_Mover->setParentPosRef (Hotspot_TM);
			_Mover->setPosRef (Hotspot_TM);
			if (_Poped && _EnabledResizer)
				_Mover->setY (options->TrackYWithTopResizer);
			else
				_Mover->setY (options->TrackY);
			_Mover->setH (options->TrackH);
			_MoverDeltaW= options->TrackW;
		}
		else
		{
			_Mover = NULL;
		}
		delCtrl ("mover");
		if (_Mover != NULL)
			addCtrl (_Mover, 0);
		invalidateCoords();
	}

	// ***************************************************************************
	void CGroupContainer::updateViewOpenState()
	{
		if (_Openable)
		{
			COptionsLayer *pLayer = getContainerOptions();
			if (_ViewOpenState == NULL)
			{
				_ViewOpenState = new CViewBitmap(CViewBase::TCtorParam());
				_ViewOpenState->setId(_Id+":open_state");
				_ViewOpenState->setParent (this);
				_ViewOpenState->setParentPos (this);
				_ViewOpenState->setParentPosRef (Hotspot_TL);
				_ViewOpenState->setPosRef (Hotspot_TL);
				_ViewOpenState->setModulateGlobalColor (getModulateGlobalColor());
				addView (_ViewOpenState);
			}
			_ViewOpenState->setX(pLayer->getValSInt32 ("open_state_offset_x"));
			_ViewOpenState->setY(pLayer->getValSInt32 ("open_state_offset_y"));

			if (_Opened)
				_ViewOpenState->setTexture (pLayer->getValStr ("open_state_tx_opened"));
			else
				_ViewOpenState->setTexture (pLayer->getValStr ("open_state_tx_closed"));
			_ViewOpenState->setActive(!_Locked);
		}
		else
		{
			_ViewOpenState = NULL;
			delView ("open_state");
		}
	}

	// ***************************************************************************
	void CGroupContainer::updateTitle()
	{
		COptionsLayer *pLayer = getContainerOptions();
		if (_TitleOpened == NULL)
		{
			switch(_TitleClass)
			{
				case TitleTextFormated:
					{
						CViewTextFormated *vtf = new CViewTextFormated(CViewBase::TCtorParam());
						vtf->setFormatString(_TitleTextOpened);
						_TitleOpened = vtf;
					}
					break;
				case TitleTextId:
				case TitleTextDynString:
					{
						CViewTextID	*vti= new CViewTextID(CViewBase::TCtorParam());
						// the title here is actually the DB path
						vti->setDBTextID(_TitleTextOpened.toString());
						vti->setDynamicString(_TitleClass==TitleTextDynString);
						_TitleOpened = vti;
					}
					break;
				default:
					_TitleOpened = new CViewText(CViewBase::TCtorParam());
			}
			_TitleOpened->setId(_Id+":titopen");
			_TitleOpened->setParent (this);
			_TitleOpened->setParentPos (this);
			_TitleOpened->setParentPosRef (Hotspot_TL);
			_TitleOpened->setPosRef (Hotspot_TL);
			_TitleOpened->setShadow (true);
			_TitleOpened->setColor (CRGBA(255,255,255,255));
			_TitleOpened->setModulateGlobalColor(getModulateGlobalColor());
			_TitleOpened->setOverExtendViewText(_TitleOverExtendViewText);
			addView (_TitleOpened);
		}

		if (_Openable)
		{
			_TitleOpened->setX (pLayer->getValSInt32 ("title_offset_openable_x"));
			_TitleOpened->setY (pLayer->getValSInt32 ("title_offset_openable_y"));
		}
		else
		{
			_TitleOpened->setX (pLayer->getValSInt32 ("title_offset_x"));
			_TitleOpened->setY (pLayer->getValSInt32 ("title_offset_y"));
		}
		_TitleOpened->setFontSize (pLayer->getValSInt32 ("title_font_size"));
		if (_TitleClass==TitleText) _TitleOpened->setText (_TitleTextOpened);
		_TitleOpened->setActive (_Opened);

		// Title when the container is closed
		if (_TitleClosed == NULL)
		{
			switch(_TitleClass)
			{
				case TitleTextFormated:
					{
						CViewTextFormated *vtf = new CViewTextFormated(CViewBase::TCtorParam());
						vtf->setFormatString(_TitleTextClosed);
						_TitleClosed = vtf;
					}
					break;
				case TitleTextId:
				case TitleTextDynString:
					{
						CViewTextID	*vti= new CViewTextID(CViewBase::TCtorParam());
						// the title here is actually the DB path
						vti->setDBTextID(_TitleTextClosed.toString());
						vti->setDynamicString(_TitleClass==TitleTextDynString);
						_TitleClosed = vti;
					}
					break;
				default:
					_TitleClosed = new CViewText(CViewBase::TCtorParam());
			}
			_TitleClosed->setId(_Id+":titclose");
			_TitleClosed->setParent (this);
			_TitleClosed->setParentPos (this);
			_TitleClosed->setParentPosRef (Hotspot_TL);
			_TitleClosed->setPosRef (Hotspot_TL);
			_TitleClosed->setShadow (true);
			_TitleClosed->setColor (CRGBA(255,255,255,255));
			_TitleClosed->setModulateGlobalColor(getModulateGlobalColor());
			_TitleClosed->setOverExtendViewText(_TitleOverExtendViewText);
			addView (_TitleClosed);
		}

		if (_Openable)
		{
			_TitleClosed->setX (pLayer->getValSInt32 ("title_offset_openable_x"));
			_TitleClosed->setY (pLayer->getValSInt32 ("title_offset_openable_y"));
		}
		else
		{
			_TitleClosed->setX (pLayer->getValSInt32 ("title_offset_x"));
			_TitleClosed->setY (pLayer->getValSInt32 ("title_offset_y"));
		}
		_TitleClosed->setFontSize (pLayer->getValSInt32 ("title_font_size"));
		if (_TitleClass==TitleText) _TitleClosed->setText (_TitleTextClosed);
		_TitleClosed->setActive(!_Opened);


	}

	// ***************************************************************************
	// bMaxH is a boolean to know if the resizer act on the content or on the child list
	void CGroupContainer::createResizer(uint index, THotSpot posRef, THotSpot type, sint32 offsetX, sint32 offsetY, bool bMaxH)
	{
		CCtrlResizer *cr = new CCtrlResizer(CViewText::TCtorParam());
		cr->setId (_Id+toString(":rz%d", (int) index));
		cr->setParent (this);
		cr->setParentPos (this);
		cr->setResizerPos(type);

		if (_LayerSetup != 0)
		{
			cr->WMin = _MinW;
			cr->WMax = _MaxW;
		}
		else
		{
			cr->WMin = _PopupMinW;
			cr->WMax = _PopupMaxW;
			cr->HMin = _PopupMinH;
			cr->HMax = _PopupMaxH;
		}

		cr->setParentPosRef (posRef);
		cr->setPosRef (posRef);

		cr->setX(offsetX);
		cr->setY(offsetY);

		cr->IsMaxH = bMaxH;

		updateResizerSize(cr);
		_Resizer[index] = cr;
		addCtrl (_Resizer[index], 0);
	}

	// ***************************************************************************
	void CGroupContainer::createResizerMaxH()
	{
		if (_LayerSetup != 0) return;
		if (_List == NULL) return;
		if (_List->getNumChildren() == 0) return;

		COptionsContainerMove *options = getMoveOptions();

		// Create corner resizer if we asked for all resizer
		if (_EnabledResizer)
		{
			if (_Resizer[1] == NULL) createResizer(1, Hotspot_TR, Hotspot_TR, 0, 0, true);
			if (_Resizer[3] == NULL) createResizer(3, Hotspot_BR, Hotspot_BR, 0, 0, true);
			if (_Resizer[5] == NULL) createResizer(5, Hotspot_BL, Hotspot_BL, 0, 0, true);
			if (_Resizer[7] == NULL) createResizer(7, Hotspot_TL, Hotspot_TL, 0, 0, true);
			_Resizer[1]->IsMaxH = true;
			_Resizer[3]->IsMaxH = true;
			_Resizer[5]->IsMaxH = true;
			_Resizer[7]->IsMaxH = true;
		}

		if (_Resizer[0] == NULL) createResizer(0, Hotspot_TL, Hotspot_TM, options->ResizerSize, 0, true);
		if (_Resizer[4] == NULL) createResizer(4, Hotspot_BR, Hotspot_BM, -options->ResizerSize, 0, true);
		_Resizer[0]->IsMaxH = true;
		_Resizer[4]->IsMaxH = true;
	}

	// ***************************************************************************
	void CGroupContainer::removeResizerMaxH()
	{
		if (_LayerSetup != 0) return;
		if (_List == NULL) return;
		if (_List->getNumChildren() != 0) return;

		for (uint i = 0; i < NumResizers; ++i)
			if ((i != 6) && (i != 2)) // 6 == right and 2 == left
				if (_Resizer[i] != NULL)
				{
					delCtrl ( toString(":rz%d", (int) i) );
					_Resizer[i] = NULL;
				}
	}

	// ***************************************************************************
	sint32 CGroupContainer::getLayer()
	{
		if (_MovingInParentList)
		{
			// keep the previous layer
			return _LayerSetup;
		}
		else
		{
			// Count Nb of Parent
			CInterfaceGroup *pIG = this;
			sint32 nNbParent = 0;
			while (pIG != NULL)
			{
				pIG = pIG->getParent();
				if (pIG != NULL)
					nNbParent++;
			}
			return (nNbParent-1)/2;
		}
	}

	// ***************************************************************************
	COptionsLayer *CGroupContainer::getContainerOptions(sint32 ls)
	{
		if (ls == -1)
			ls = _LayerSetup;

		string sTmp;
		const string *sLayerName;
		if (_OptionsName.empty())
		{
			nlassert((uint32)ls<NumLayerName);
			sLayerName = &_OptionLayerName[ls];
		}
		else
		{
			sTmp = _OptionsName;
			sLayerName = &sTmp;
		}

		COptionsLayer *pLayer = (COptionsLayer*)CWidgetManager::getInstance()->getOptions(*sLayerName);
		nlassert(pLayer != NULL);
		return pLayer;
	}

	// ***************************************************************************
	void	CGroupContainer::setContent (CInterfaceGroup *pC)
	{
		if (_Content != NULL)
			delGroup (_Content);
		_Content = pC;
		if (_Content)
		{
			_Content->setId (_Id+":content");
			_Content->setActive (false);
			_Content->setParentPosRef (Hotspot_TL);
			_Content->setPosRef (Hotspot_TL);
			_Content->setParent (this);
			addGroup (_Content);
		}
	}

	// ***************************************************************************
	std::string		CGroupContainer::getTitle () const
	{
		return _TitleTextOpened.toString();
	}

	// ***************************************************************************
	void			CGroupContainer::setTitle (const std::string &title)
	{
		if (_Localize)	setUCTitle (CI18N::get(title));
		else			setUCTitle (title);
	}

	// ***************************************************************************
	std::string		CGroupContainer::getTitleOpened () const
	{
		return _TitleTextOpened.toString();
	}

	// ***************************************************************************
	void			CGroupContainer::setTitleOpened (const std::string &title)
	{
		if (_Localize)	setUCTitleOpened (CI18N::get(title));
		else			setUCTitleOpened (title);
	}

	// ***************************************************************************
	std::string		CGroupContainer::getTitleClosed () const
	{
		return _TitleTextClosed.toString();
	}

	// ***************************************************************************
	void			CGroupContainer::setTitleClosed (const std::string &title)
	{
		if (_Localize)	setUCTitleClosed (CI18N::get(title));
		else			setUCTitleClosed (title);
	}

	// ***************************************************************************
	void CGroupContainer::setUCTitleOpened(const ucstring &title)
	{
		_TitleTextOpened = title;
		if (_TitleOpened != NULL)
			_TitleOpened->setText (title);
		invalidateCoords();
	}

	// ***************************************************************************
	void CGroupContainer::setUCTitleClosed(const ucstring &title)
	{
		_TitleTextClosed = title;
		if (_TitleClosed != NULL)
			_TitleClosed->setText (_TitleTextClosed);
		invalidateCoords();
	}

	// ***************************************************************************
	void CGroupContainer::setUCTitle(const ucstring &title)
	{
		setUCTitleOpened(title);
		setUCTitleClosed(title);
	}

	// ***************************************************************************
	ucstring CGroupContainer::getUCTitle () const
	{
		return getUCTitleOpened();
	}

	// ***************************************************************************
	ucstring CGroupContainer::getUCTitleOpened () const
	{
		return _TitleTextOpened;
	}

	// ***************************************************************************
	ucstring CGroupContainer::getUCTitleClosed () const
	{
		return _TitleTextClosed;
	}

	// ***************************************************************************
	void	CGroupContainer::launch ()
	{
		if (_OpenAtStart)
			open();

		CInterfaceGroup::launch();
	}

	// ***************************************************************************
	void CGroupContainer::setActive (bool state)
	{
		if(state != getActive() && getLayer()==0)
		{
			if (state)
				CWidgetManager::getInstance()->setTopWindow(this);
			else
				CWidgetManager::getInstance()->setBackWindow(this);
		}
		CAHManager::getInstance()->submitEvent((state?"show:":"hide:")+getId());

		CInterfaceGroup::setActive(state);
	}

	// ***************************************************************************
	/*bool CGroupContainer::isWindowUnder (sint32 x, sint32 y)
	{
		bool bGrayed = false;
		CGroupContainer *pSon = _ModalSon;
		if ((pSon != NULL) && pSon->getActive())
			bGrayed = true;

		if (bGrayed) return NULL;
		return CInterfaceGroup::isWindowUnder(x,y);
	}*/

	// ***************************************************************************
	bool CGroupContainer::getViewsUnder (sint32 x, sint32 y, sint32 clipX, sint32 clipY, sint32 clipW, sint32 clipH, std::vector<CViewBase*> &vVB)
	{
		bool bGrayed = isGrayed();
		if (bGrayed) return false;
		return CInterfaceGroup::getViewsUnder(x, y, clipX, clipY, clipW, clipH, vVB);
	}

	// ***************************************************************************
	bool CGroupContainer::getCtrlsUnder (sint32 x, sint32 y, sint32 clipX, sint32 clipY, sint32 clipW, sint32 clipH, std::vector<CCtrlBase*> &vICL)
	{
		bool bGrayed = isGrayed();
		if (bGrayed) return false;
		return CInterfaceGroup::getCtrlsUnder(x,y,clipX,clipY,clipW,clipH,vICL);
	}

	// ***************************************************************************
	void CGroupContainer::popupCurrentPos()
	{
		if (!_Popable && !_MovableInParentList)
		{
			nlwarning("<CGroupContainer::popup> The window is not popable or cannot move in parent list.");
			return;
		}
		if (_LayerSetup == 0)
		{
			nlwarning("<CGroupContainer::popup> The window is already poped.");
			return;
		}
		if (!_Parent || !_Parent->getParent())
		{
			nlwarning("<CGroupContainer::popup> Window has not parent.");
			return;
		}
		// remove the group from its father
		CGroupContainer *parentContainer = dynamic_cast<CGroupContainer *>(_Parent->getParent());
		if (!parentContainer)
		{
			nlwarning("<CGroupContainer::popup> Container is not son of another container");
			return;
		}
		_OldFatherContainer = parentContainer;
		sint32 eltIndex = parentContainer->_List->getElementIndex(this);
		if (eltIndex == -1)
		{
			nlwarning("<CGroupContainer::popup> Can't get index in owner group");
			return;
		}
		_InsertionOrder = parentContainer->_List->getOrder(eltIndex);
		parentContainer->detachContainer(this);

		parentContainer->_PopedCont.push_back(this);

		// put at the base of hierarchy
		CInterfaceGroup *parent = _Parent;
		if (!parent) return;
		while (parent->getParent())
		{
			parent = parent->getParent();
		}
		_Parent = parent;
		_ParentPos = parent;

		CWidgetManager::getInstance()->makeWindow(this);
		CWidgetManager::getInstance()->setTopWindow(this);
		CWidgetManager::getInstance()->clearViewUnders();
		CWidgetManager::getInstance()->clearCtrlsUnders();

		// update coords (put coords in world)
		setX(getXReal());
		setY(getYReal() + getHReal());
		setParentPosRef(Hotspot_BL);
		setPosRef(Hotspot_TL);

		// clamp coords
		// width
		sint32 w = getW();
		clamp(w, _PopupMinW, _PopupMaxW);
		setW(w);

		invalidateCoords();

		// setup the new controls
		_Poped = true;
		_OpenedBeforePopup = _Opened;
		_Opened = true;
		disableBlink();
		setup();
	}

	// ***************************************************************************
	void CGroupContainer::popin(sint32 insertPos /* = -1 */, bool putBackInFatherContainer /*= true*/)
	{
		if (!_OldFatherContainer)
		{
			nlwarning("<popin> The window wasn't previously attached.(%s)",this->_Id.c_str());
			return;
		}

		if (!_Popable && !_MovableInParentList)
		{
			nlwarning("<popin> The window is not popable or cannot move in parent list.(%s)",this->_Id.c_str());
			return;
		}
		if (!_MovingInParentList && _LayerSetup != 0)
		{
			nlwarning("<popin> The window should be in layer 0.(%s)",this->_Id.c_str());
			return;
		}
		if (!_Parent)
		{
			nlwarning("<popin> The window has no parent.(%s)",this->_Id.c_str());
			return;
		}

		touch();
		_List->setOfsY(0);

		_MovingInParentList = false;
		CWidgetManager::getInstance()->unMakeWindow(this);
		CWidgetManager::getInstance()->clearViewUnders();
		CWidgetManager::getInstance()->clearCtrlsUnders();
		_Parent = NULL;
		_ParentPos = NULL;
		std::vector<CGroupContainer *>::iterator it = std::find(_PopedCont.begin(), _PopedCont.end(), this);
		if (it != _PopedCont.end())
		{
			// replace by last element
			*it = _PopedCont.back();
			_PopedCont.pop_back();
		}
		if (putBackInFatherContainer)
		{
			bool active = getActive();
			_Poped = false;
			if (insertPos == -1)
			{
				_OldFatherContainer->attachContainer(this, _InsertionOrder);
			}
			else
			{
				if (!_OldFatherContainer->attachContainerAtIndex(this, insertPos))
				{
					nlwarning("Couldn't attach to previous container");
					return;
				}
			}
			setActive(active);
			_OldFatherContainer = NULL;
			if (_OpenWhenPopup)
			{
				setOpen(false);
			}
			else
			{
				setOpen(_OpenedBeforePopup);
			}
		}

		invalidateCoords();

		_OldFatherContainer = NULL;
		setup();
	}

	// ***************************************************************************
	void CGroupContainer::enableBlink(uint numBlinks /*=0*/)
	{
		_Blinking = true;
		_NumBlinks = numBlinks;
		_BlinkDT = 0;
		_BlinkState = true;
	}

	// ***************************************************************************
	void CGroupContainer::disableBlink()
	{
		_Blinking = false;
		_NumBlinks = 0;
		_BlinkDT = 0;
		_BlinkState = false;
	}


	// ***************************************************************************
	void CGroupContainer::setMovingInParentList(bool enable)
	{
		_MovingInParentList = enable;
	}

	// ***************************************************************************
	void CGroupContainer::popup()
	{
		if (_Poped) return;
		touch();
		setHighLighted(false);
		// pop the window
		popupCurrentPos();
		if (getPopupW() != -1)
		{
			setX(getPopupX());
			setY(getPopupY());
			setW(getPopupW());
			setH(getPopupH());
		}
		else
		{
			setW(getRefW()); // Do not know what we need to do that ...
		}
		invalidateCoords(2);
	}

	// ***************************************************************************
	COptionsContainerMove *CGroupContainer::getMoveOptions()
	{
		static		NLMISC::CRefPtr<COptionsContainerMove>     moveOptions;
		if (moveOptions) return moveOptions;
		moveOptions = (COptionsContainerMove *) CWidgetManager::getInstance()->getOptions("container_move_opt");
		return moveOptions;
	}


	// ***************************************************************************
	// Actions Handlers
	// ***************************************************************************

	// ***************************************************************************
	class CICOpen : public IActionHandler
	{
	public:
		virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
		{
			CInterfaceGroup *pIG = pCaller->getParent();
			if (pIG == NULL) return;
			CGroupContainer *pIC = dynamic_cast<CGroupContainer*>(pIG);
			if (pIC == NULL) return;
			pIC->open();
		}
	};
	REGISTER_ACTION_HANDLER (CICOpen, "ic_open");

	// ***************************************************************************
	class CICClose : public IActionHandler
	{
	public:
		virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
		{
			CInterfaceGroup *pIG = pCaller->getParent();
			if (pIG == NULL) return;
			CGroupContainer *pIC = dynamic_cast<CGroupContainer*>(pIG);
			if (pIC == NULL) return;
			pIC->close();
		}
	};
	REGISTER_ACTION_HANDLER (CICClose, "ic_close");

	// ***************************************************************************
	class CICDeactive : public IActionHandler
	{
	public:
		virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
		{
			CInterfaceGroup *pIG = pCaller->getParent();
			if (pIG == NULL) return;
			CGroupContainer *pIC = dynamic_cast<CGroupContainer*>(pIG);
			if (pIC == NULL) return;
			if (pIC->isLocked()) return;

			// check if the window can be really closed
			CGroupContainer::_ValidateCanDeactivate = true;
			if (!pIC->getAHOnDeactiveCheck().empty())
			{
				CAHManager::getInstance()->runActionHandler(pIC->getAHOnDeactiveCheck(), pCaller, pIC->getAHOnDeactiveCheckParams());
			}

			if (CGroupContainer::_ValidateCanDeactivate)
			{
				// send close button msg
				if (!pIC->getAHOnCloseButton().empty())
				{
					CAHManager::getInstance()->runActionHandler(pIC->getAHOnCloseButton(), pCaller, pIC->getAHOnCloseButtonParams());
				}
				CWidgetManager::getInstance()->setBackWindow(pIC);
				pIC->setActive(false);
			}
		}
	};
	REGISTER_ACTION_HANDLER (CICDeactive, "ic_deactive");

	// ***************************************************************************
	class CICPopup : public IActionHandler
	{
	public:
		virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
		{
			CInterfaceGroup *pIG = pCaller->getParent();
			if (pIG == NULL) return;
			CGroupContainer *pIC = dynamic_cast<CGroupContainer*>(pIG);
			if (pIC == NULL) return;
			if (pIC->isLocked()) return;
			//
			pIC->popup();
			//

			CWidgetManager::getInstance()->setCapturePointerLeft(NULL);
			CWidgetManager::getInstance()->setCapturePointerRight(NULL);
		}
	};
	REGISTER_ACTION_HANDLER (CICPopup, "ic_popup");

	// ***************************************************************************
	class CICPopin : public IActionHandler
	{
	public:
		virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
		{
			CInterfaceGroup *pIG = pCaller->getParent();
			if (pIG == NULL) return;
			CGroupContainer *pIC = dynamic_cast<CGroupContainer*>(pIG);
			if (pIC == NULL) return;
			if (pIC->isLocked()) return;
			// memorize popup position
			pIC->setPopupX(pIC->getX());
			pIC->setPopupY(pIC->getY());
			pIC->setPopupW(pIC->getW());
			pIC->setPopupH(pIC->getH());
			//
			pIC->popin();

			CWidgetManager::getInstance()->setCapturePointerLeft(NULL);
			CWidgetManager::getInstance()->setCapturePointerRight(NULL);
		}
	};
	REGISTER_ACTION_HANDLER (CICPopin, "ic_popin");

	// ***************************************************************************
	class CICLock : public IActionHandler
	{
		virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
		{
			CInterfaceGroup *pIG = pCaller->getParent();
			if (pIG == NULL) return;
			CGroupContainer *pIC = dynamic_cast<CGroupContainer*>(pIG);
			if (pIC == NULL) return;
			pIC->setLocked(!pIC->isLocked());
		}
	};
	REGISTER_ACTION_HANDLER(CICLock, "ic_lock");

	// ***************************************************************************
	class CICHelp : public IActionHandler
	{
		virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
		{
			// get the container to get help
			if(!pCaller)
				return;
			CGroupContainer *pIC = dynamic_cast<CGroupContainer*>(pCaller->getRootWindow());
			if (pIC == NULL) return;

			// if found the help page
			const std::string	&helpPage= pIC->getHelpPage();
			if(!helpPage.empty())
			{

				// open the web browser, and point to the page
				CAHManager::getInstance()->runActionHandler("launch_help", NULL, "url=" + helpPage);
			}
		}
	};
	REGISTER_ACTION_HANDLER(CICHelp, "ic_help");

	// ***************************************************************************
	CGroupContainer *CGroupContainer::getFatherContainer() const
	{
		if (_Parent && _Parent->getParent())
		{
			return dynamic_cast<CGroupContainer *>(_Parent->getParent());
		}
		return NULL;
	}

	// ***************************************************************************
	CGroupContainer *CGroupContainer::getProprietaryContainer() const
	{
		if (_Parent && _Parent->getParent())
		{
			return dynamic_cast<CGroupContainer *>(_Parent->getParent());
		}
		else
		{
			return _OldFatherContainer;
		}
	}

	// ***************************************************************************
	void CGroupContainer::setOpenable(bool openable)
	{
	//	COptionsLayer *pLayer = getContainerOptions();
	//	COptionsContainerMove *options = getMoveOptions();
		_Openable = openable;

		updateTitle();
		updateViewOpenState();
		updateMover();
	}

	// ***************************************************************************
	void CGroupContainer::rollOverAlphaUp()
	{

		CViewPointerBase *vp = CWidgetManager::getInstance()->getPointer();
		float speed = CWidgetManager::getInstance()->getAlphaRolloverSpeed();
		if (!isIn(vp->getX(), vp->getY()))
		{
			const CWidgetManager::SInterfaceTimes &times = CWidgetManager::getInstance()->getInterfaceTimes();

			_CurrentRolloverAlphaContainer += (float) (speed * times.frameDiffMs);
			_CurrentRolloverAlphaContainer = std::min(1.f, _CurrentRolloverAlphaContainer);

			_CurrentRolloverAlphaContent += (float) (speed * times.frameDiffMs);
			_CurrentRolloverAlphaContent = std::min(1.f, _CurrentRolloverAlphaContent);
		}
	}

	// ***************************************************************************
	void CGroupContainer::forceRolloverAlpha()
	{
		_CurrentRolloverAlphaContent = hasKeyboardFocus() ? 1.0f : 0.f;
		_CurrentRolloverAlphaContainer = hasKeyboardFocus() ? 1.0f : 0.f;
	}

	// ***************************************************************************
	bool CGroupContainer::hasKeyboardFocus() const
	{

		if (CWidgetManager::getInstance()->getCaptureKeyboard() != NULL)
		{
			const CGroupEditBox *geb = dynamic_cast<const CGroupEditBox *>(CWidgetManager::getInstance()->getCaptureKeyboard());
			if (geb)
			{
				const CInterfaceGroup *gr = geb->getParent();
				while(gr)
				{
					if (gr == this)
					{
						return true;
					}
					gr = gr->getParent();
				}
			}
		}
		return false;
	}

	// ***************************************************************************
	void CGroupContainer::setLockable(bool lockable)
	{
		if (lockable == _Lockable) return;
		setup();
	}

	// ***************************************************************************
	void CGroupContainer::setLocked(bool locked)
	{
		if (!_Lockable)
		{
			nlwarning("Container is not lockable");
			return;
		}
		if (locked == _Locked) return;
		_Locked = locked;
		if (_LayerSetup == 0)
		{
			updateRightButton();
			updateHelpButton();
			if (_ViewOpenState != NULL)	_ViewOpenState->setActive(!_Locked);
			if (_Mover != NULL)	_Mover->setActive(!_Locked);
		}
	}

	// ***************************************************************************
	std::string		CGroupContainer::getTitleColorAsString() const
	{
		// return one of the title opened....
		if(!_TitleOpened)
			return std::string();

		return _TitleOpened->getColorAsString();
	}

	// ***************************************************************************
	void			CGroupContainer::setTitleColorAsString(const std::string &col)
	{
		// Set both colors
		if(_TitleOpened)
			_TitleOpened->setColorAsString(col);
		if(_TitleClosed)
			_TitleClosed->setColorAsString(col);
	}

	// ***************************************************************************
	void CGroupContainer::setModalParentList (const std::string &name)
	{
		_ModalParentNames = name;

		// can have multiple parent
		vector<string>		modalParents;
		NLMISC::splitString(name, "|", modalParents);
		// add each of them (if possible)
		for(uint i=0;i<modalParents.size();i++)
		{
			CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(modalParents[i]));
			if (pGC == NULL)
				pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:"+modalParents[i]));
			if (pGC == NULL)
				nlwarning("<setModalParentList> not found %s",modalParents[i].c_str());
			else
				addModalParent (pGC);
		}
	}

	// ***************************************************************************
	void CGroupContainer::addModalParent (CGroupContainer *pParent)
	{
		if(pParent==NULL) return;
		// Look if parent not already added
		for(uint i=0;i<_ModalParents.size();++i)
			if(_ModalParents[i] == pParent)
				return;
		// add me to the parent
		pParent->addModalSon(this);
		_ModalParents.push_back(pParent);
	}

	// ***************************************************************************
	void CGroupContainer::addModalSon (CGroupContainer *pSon)
	{
		if (pSon == NULL) return;
		// Look if the son not already added
		for (uint i = 0; i < _ModalSons.size(); ++i)
			if (_ModalSons[i] == pSon)
				return;
		_ModalSons.push_back(pSon);
	}

	// ***************************************************************************
	bool CGroupContainer::checkIfModal(const NLGUI::CEventDescriptor& event)
	{
		bool bRet = true;
		if (event.getType() == NLGUI::CEventDescriptor::mouse)
		{
			const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;
			if ((eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftdown) ||
				(eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightdown))
			{
				bRet = blinkAllSons();
			}
			// Additionaly, if it is a UP, don't blink, but return false if some son active
			if (bRet && (
				(eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup) ||
				(eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightup))
			   )
			{
				bRet= !isGrayed();
			}
		}
		return bRet;
	}

	// ***************************************************************************
	bool CGroupContainer::isGrayed() const
	{
		bool bGrayed = false;
		for (uint i = 0; i < _ModalSons.size(); ++i)
		{
			CGroupContainer *pSon = _ModalSons[i];
			if (pSon->getActive())
				bGrayed = true;
		}
		return bGrayed;
	}

	// ***************************************************************************
	bool CGroupContainer::blinkAllSons()
	{
		bool bRet = true;
		vector<CGroupContainer*> allSons = _ModalSons;
		uint i,j;
		// Recurs all sons (because allSons grow while sons are added). NB: if there is a graph, it will freeze....
		for (i = 0; i < allSons.size(); ++i)
		{
			CGroupContainer *pSon = allSons[i];
			for (j = 0; j < pSon->_ModalSons.size(); ++j)
				allSons.push_back(pSon->_ModalSons[j]);
		}
		// Then for all sons and descendants, blink
		for (i = 0; i < allSons.size(); ++i)
		{
			CGroupContainer *pSon = allSons[i];
			if (pSon->getActive())
			{
				pSon->enableBlink(3);
				bRet = false;
			}
		}
		return bRet;
	}


	// ***************************************************************************
	void CGroupContainer::forceOpen()
	{
		// Force open the container.
		open();
		// Ensure it is Active too
		setActive(true);
	}

	// ***************************************************************************
	void CGroupContainer::backupPosition()
	{
		_BackupX = getX();
		_BackupY = getY();
		_PositionBackuped = true;
	}

	// ***************************************************************************
	void CGroupContainer::restorePosition()
	{
		if (!_PositionBackuped) return;
		setX(_BackupX);
		setY(_BackupY);
		_PositionBackuped = false;
	}

	// ***************************************************************************
	bool CGroupContainer::getTouchFlag(bool clearFlag) const
	{
		bool touchFlag = _TouchFlag;
		if (clearFlag)
		{
			_TouchFlag = false;
		}
		return touchFlag;
	}

	// ***************************************************************************
	void CGroupContainer::setBackupPosition(sint32 x, sint32 y)
	{
		_BackupX = x;
		_BackupY = y;
		_PositionBackuped = true;
	}

	// ***************************************************************************
	void CGroupContainer::setPopupMinW(sint32 minW)
	{
		_PopupMinW = minW;
		// TODO : avoid this stupid copy (issue in CCtrResizer..)
		for(uint k = 0; k < NumResizers; ++k)
		{
			if (_Resizer[k]) _Resizer[k]->WMin = minW;
		}
	}

	// ***************************************************************************
	void CGroupContainer::setPopupMaxW(sint32 maxW)
	{
		_PopupMaxW = maxW;
		// TODO : avoid this stupid copy (issue in CCtrResizer..)
		for(uint k = 0; k < NumResizers; ++k)
		{
			if (_Resizer[k]) _Resizer[k]->WMax = maxW;
		}
	}

	// ***************************************************************************
	void	CGroupContainer::setPopupMinH(sint32 minH)
	{
		_PopupMinH = minH;
		// TODO : avoid this stupid copy (issue in CCtrResizer..)
		for(uint k = 0; k < NumResizers; ++k)
		{
			if (_Resizer[k]) _Resizer[k]->HMin = minH;
		}
	}

	// ***************************************************************************
	void	CGroupContainer::setPopupMaxH(sint32 maxH)
	{
		_PopupMaxH = maxH;
		// TODO : avoid this stupid copy (issue in CCtrResizer..)
		for(uint k = 0; k < NumResizers; ++k)
		{
			if (_Resizer[k]) _Resizer[k]->HMax = maxH;
		}
	}
	// ***************************************************************************
	int CGroupContainer::luaSetHeaderColor(CLuaState &ls)
	{
		const char *funcName = "setHeaderColor";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
		_HeaderColor.link(ls.toString(1));
		return 0;
	}

	// ***************************************************************************
	CRGBA	CGroupContainer::getDrawnHeaderColor () const
	{
		CRGBA c = CRGBA(255,255,255,255);

		// Display the header in white if we are the last clicked window
		if (CWidgetManager::getInstance()->getTopWindow(CWidgetManager::getInstance()->getLastTopWindowPriority()) != static_cast<const CInterfaceGroup*>(this))
		{
			if (_HeaderColor.getNodePtr() != NULL)
				c = _HeaderColor.getRGBA();
			if (isGrayed())
			{
				c.R = c.R / 2;
				c.G = c.G / 2;
				c.B = c.B / 2;
			}
			c.A = 255;
		}

		return c;
	}

	// ***************************************************************************
	void	CGroupContainer::setHelpPage(const std::string &newPage)
	{
		_HelpPage= newPage;
	}

	// ***************************************************************************
	void CGroupContainer::requireAttention()
	{
		if (getActive())
		{
			// Window have headers opened => blink it if is not the top window
			if (isOpen())
			{
				if (getId() != CWidgetManager::getInstance()->getTopWindow()->getId())
				{
					enableBlink(3);
				}
			}
			// Window have headers closed => change color of header
			else
			{
				setHeaderColor("UI:SAVE:WIN:COLORS:INFOS");
			}
		}
		else
		{
			// Must open this window everytime someone tell something on it
			setActive(true);
		}
	}


	// ***************************************************************************
	int CGroupContainer::luaBlink(CLuaState &ls)
	{
		const char *funcName = "blink";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		enableBlink((uint) ls.toNumber(1));
		return 0;
	}

	// ***************************************************************************
	void CGroupContainer::setRightButtonEnabled(bool enabled)
	{
		if (_EnabledRightButton == enabled) return;
		_EnabledRightButton = enabled;
		updateRightButton();
	}

	// ***************************************************************************
	void CGroupContainer::setContentYOffset(sint32 value)
	{
		#ifdef  NL_DEBUG
			nlassert(value <= 127 && value >= -128);
		#endif
		if (value > 127 || value < -128)
		{
			// for lua exported value, let know the user that there was some problem
			throw NLMISC::Exception("y_offset must be in the [-128, 127] range");
		}
		_ContentYOffset = (sint8) value;
		invalidateCoords();
	}


}

