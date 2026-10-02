// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2015  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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
#include "nel/gui/group_scrolltext.h"
#include "nel/gui/group_list.h"
#include "nel/gui/view_text.h"
#include "nel/gui/ctrl_scroll.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/view_renderer.h"
#include "nel/misc/i18n.h"
#include "nel/gui/widget_manager.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

NLMISC_REGISTER_OBJECT(CViewBase, CGroupScrollText, std::string, "scroll_text");

namespace NLGUI
{

	//========================================================================
	CGroupScrollText::CGroupScrollText(const TCtorParam &param) :
											CInterfaceGroup(param),
											_List(NULL),
											_ScrollBar(NULL),
											_ButtonAdd(NULL),
											_ButtonSub(NULL),
											_ButtonNewest(NULL),
											_ButtonUnread(NULL),
											_UnreadSeen(false),
											_LastCheckMs(0),
											_Settuped(false),
											_InvertScrollBar(true),
											_ListHeight(0),
											_Scrolling(false),
											_ScrollDistance(0),
											_ClockMsgEventRegistered(false),
											_StartHeight(0),
											_EllapsedTime(0)
	{
		_IsGroupScrollText = true;
	}

	//========================================================================
	CGroupScrollText::~CGroupScrollText()
	{
	}

	std::string CGroupScrollText::getProperty( const std::string &name ) const
	{
		if( name == "invert_scroll_bar" )
			return NLMISC::toString( _InvertScrollBar );
		else
			return CInterfaceGroup::getProperty( name );
	}

	void CGroupScrollText::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "invert_scroll_bar" )
		{
			bool b;
			if( NLMISC::fromString( value, b ) )
				_InvertScrollBar = b;
			return;
		}
		else
			CInterfaceGroup::setProperty( name, value );
	}


	xmlNodePtr CGroupScrollText::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CInterfaceGroup::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "scroll_text" );
		xmlSetProp( node, BAD_CAST "invert_scroll_bar", BAD_CAST NLMISC::toString( _InvertScrollBar ).c_str() );

		return node;
	}

	//========================================================================
	bool CGroupScrollText::parse(xmlNodePtr cur,CInterfaceGroup *parentGroup)
	{
		if(!CInterfaceGroup::parse(cur, parentGroup))
			return false;
		CXMLAutoPtr ptr;

		// invert scroll bar?
		ptr = xmlGetProp (cur, (xmlChar*)"invert_scroll_bar");
		if(ptr)		_InvertScrollBar= convertBool(ptr);

		return true;
	}

	//========================================================================
	void CGroupScrollText::updateCoords()
	{
		CInterfaceGroup::updateCoords();
		if (!_Settuped) setup();
		updateScrollBar();
		// re-update of scrollbar
		if(_ScrollBar)
			_ScrollBar->updateCoords();
	}

	//========================================================================
	void CGroupScrollText::updateScrollBar()
	{
		if (_List && _ScrollBar)
		{
			if (_List->getHReal() < _List->getMaxHReal())
			{
				_ScrollBar->setActive(false);
			}
			else
			{
				_ScrollBar->setActive(true);
			}
		}
	}

	//========================================================================
	void CGroupScrollText::checkCoords ()
	{
		// update scrollbar if necessary
		if (_List)
		{
			if (_List->getH() != _ListHeight) // see if the scrollbar should be updated
			{
				invalidateCoords();
				_ListHeight = _List->getH();
			}
			if (_ButtonNewest || _ButtonUnread)
				updateUnread();
		}
		CInterfaceGroup::checkCoords();
	}

	//========================================================================
	bool CGroupScrollText::isAtNewest() const
	{
		// The list is bottom aligned: a negative offset scrolls back in time.
		return !_List || _List->getHReal() <= _List->getMaxHReal() || _List->getOfsY() >= 0;
	}

	//========================================================================
	bool CGroupScrollText::isLineVisible(const CViewBase *line) const
	{
		return line->getYReal() >= _List->getYReal() &&
			line->getYReal() + line->getHReal() <= _List->getYReal() + _List->getMaxHReal();
	}

	//========================================================================
	void CGroupScrollText::updateUnread()
	{
		// A skipped frame means the list was not displayed in between.
		const CWidgetManager::SInterfaceTimes &times = CWidgetManager::getInstance()->getInterfaceTimes();
		const bool firstCheck = _LastCheckMs == 0;
		const bool displayed = _LastCheckMs == times.lastFrameMs;
		_LastCheckMs = times.thisFrameMs;

		const uint count = _List->getNumChildren();
		CViewBase *last = count ? _List->getChild(count - 1) : NULL;
		if (last != _LastLine)
		{
			if (displayed && isAtNewest())
				_FirstUnread = NULL;
			else if (!firstCheck && (_FirstUnread == NULL || _UnreadSeen))
			{
				uint first = count;
				while (first > 0 && _List->getChild(first - 1) != _LastLine)
					--first;
				markUnread(first < count ? _List->getChild(first) : NULL);
			}
			_LastLine = last;
		}
		if (displayed && _FirstUnread && isLineVisible(_FirstUnread))
			_UnreadSeen = true;

		if (_ButtonNewest)
			_ButtonNewest->setActive(!isAtNewest());
		if (_ButtonUnread)
			_ButtonUnread->setActive(_FirstUnread != NULL && !_UnreadSeen && !isLineVisible(_FirstUnread));
	}

	//========================================================================
	void CGroupScrollText::markUnread(CViewBase *line)
	{
		// Lines read before stay above the new marker.
		if (_FirstUnread == NULL || _UnreadSeen)
		{
			_FirstUnread = line;
			_UnreadSeen = false;
		}
	}

	//========================================================================
	void CGroupScrollText::scrollToNewest()
	{
		if (_List && _ScrollBar && _List->getNumChildren())
			_ScrollBar->ensureVisible(_List->getChild(_List->getNumChildren() - 1), Hotspot_Bx, Hotspot_Bx);
	}

	//========================================================================
	void CGroupScrollText::scrollToUnread()
	{
		if (_ScrollBar && _FirstUnread)
			_ScrollBar->ensureVisible(_FirstUnread, Hotspot_Tx, Hotspot_Tx);
	}

	//========================================================================
	void CGroupScrollText::draw()
	{
		CInterfaceGroup::draw();
		// Line above the first unread message.
		if (_List && _FirstUnread && isLineVisible(_FirstUnread))
		{
			CViewRenderer &rVR = *CViewRenderer::getInstance();
			NLMISC::CRGBA color(255, 255, 255, CWidgetManager::getInstance()->getGlobalColorForContent().A / 2);
			rVR.drawRotFlipBitmap(_RenderLayer, _List->getXReal(), _FirstUnread->getYReal() + _FirstUnread->getHReal(),
				_List->getWReal(), 1, 0, false, rVR.getBlankTextureId(), color);
		}
	}

	//========================================================================
	void CGroupScrollText::clearViews()
	{
		CInterfaceGroup::clearViews();
	}

	//========================================================================
	bool CGroupScrollText::handleEvent(const NLGUI::CEventDescriptor &event)
	{
		if (event.getType() == NLGUI::CEventDescriptor::mouse)
		{
			const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;
			//
			if (_List && _ScrollBar)
			{
				if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousewheel)
				{
					if (isIn(eventDesc.getX(), eventDesc.getY()))
					{
						// limit scroll to 100px with single wheel event
						sint32 h = std::min(100, _List->getMaxHReal() / 2);
						if (h == 0) h = 1;

						smoothScrollY(- eventDesc.getWheel() * h);
						return true;
					}
				}
			}
		}

		if (event.getType() == NLGUI::CEventDescriptor::system)
		{
			if (_Scrolling && _ScrollBar)
			{
				float dy = _ScrollDistance / 4;
				if ((sint32) dy != 0)
				{
					_ScrollBar->moveTargetY(dy);
					_ScrollDistance -= dy;
				}
				else
				{
					_Scrolling = false;
					if (_ClockMsgEventRegistered)
					{
						_ClockMsgEventRegistered = false;
						CWidgetManager::getInstance()->unregisterClockMsgTarget(this);
					}
				}
			}
		}

		if (CInterfaceGroup::handleEvent(event)) return true;
		return false;
	}

	//========================================================================
	void CGroupScrollText::smoothScrollY(sint32 dy)
	{
		if (!_Scrolling)
		{
			_Scrolling = true;
			_ScrollDistance = 0;

			// register for clock tick event if not already done
			CWidgetManager *pWM = CWidgetManager::getInstance();
			if (!pWM->isClockMsgTarget(this))
			{
				pWM->registerClockMsgTarget(this);
				_ClockMsgEventRegistered = true;
			}
		}

		_ScrollDistance += dy;
	}

	//========================================================================
	void CGroupScrollText::setup()
	{
		// bind to the controls
		_ScrollBar = dynamic_cast<CCtrlScroll *>(CInterfaceGroup::getCtrl("scroll_bar"));
		_ButtonAdd = dynamic_cast<CCtrlBaseButton *>(CInterfaceGroup::getCtrl("button_add"));
		_ButtonSub = dynamic_cast<CCtrlBaseButton *>(CInterfaceGroup::getCtrl("button_sub"));
		_ButtonNewest = dynamic_cast<CCtrlBaseButton *>(CInterfaceGroup::getCtrl("button_newest"));
		_ButtonUnread = dynamic_cast<CCtrlBaseButton *>(CInterfaceGroup::getCtrl("button_unread"));
		_List	   = dynamic_cast<CGroupList *>(CInterfaceGroup::getGroup("text_list"));

		if(_ScrollBar == NULL)
			nlwarning("<setup> scroll bar 'scroll_bar' missing or bad type.(%s)",this->_Id.c_str());
		// Add and sub button are not required
		/*
		if(buttonAdd == NULL)
			nlwarning("Interface: CGroupScrollText: button 'button_add' missing or bad type");
		if(buttonSub == NULL)
			nlwarning("Interface: CGroupScrollText: button 'button_sub' missing or bad type");
		*/
		if(_List == NULL)
			nlwarning("<setup> group list 'text_list' missing or bad type");

		// actions
		if (_ButtonAdd) _ButtonAdd->setActionOnClockTick("gst_add");
		if (_ButtonSub) _ButtonSub->setActionOnClockTick("gst_sub");
		if (_ButtonNewest) _ButtonNewest->setActionOnLeftClick("gst_newest");
		if (_ButtonUnread) _ButtonUnread->setActionOnLeftClick("gst_unread");

		// bind the scrollbar to the list
		if (_ScrollBar)
		{
			_ScrollBar->setTarget(_List);
			//_ScrollBar->setInverted(_InvertScrollBar);
		}
		_Settuped = true;
	}

	//========================================================================
	void CGroupScrollText::elementCaptured(CCtrlBase *capturedElement)
	{
		if (capturedElement == _ButtonAdd || capturedElement == _ButtonSub)
		{
			// reset the counters for increase
			_EllapsedTime = 0;
			_StartHeight = getH();
		}
	}

	// ***************************************************************************
	// ***************************************************************************
	// Actions Handlers
	// ***************************************************************************
	// ***************************************************************************


	// ***************************************************************************
	class CSTUp : public IActionHandler
	{
	public:
		virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
		{
			const CWidgetManager::SInterfaceTimes &times = CWidgetManager::getInstance()->getInterfaceTimes();

			CGroupScrollText *pST = dynamic_cast<CGroupScrollText*>(pCaller->getParent());
			if (pST == NULL) return;
			if (pST->getList() == NULL) return;
			// get the font height from the text template of the list
			const CViewText *vt = pST->getList()->getTextTemplatePtr();
			if (!vt) return;
			pST->_EllapsedTime += times.frameDiffMs;
	//		pST->setH(std::min((sint32) pST->getMaxHeight(), (sint32) (pST->_StartHeight + pST->_EllapsedTime / 9)));
			pST->setH((sint32) (pST->_StartHeight + pST->_EllapsedTime / 9));
			pST->invalidateCoords();
		}
	};
	REGISTER_ACTION_HANDLER (CSTUp, "gst_add");

	// ***************************************************************************
	class CSTDown : public IActionHandler
	{
	public:
		virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
		{
			const CWidgetManager::SInterfaceTimes &times = CWidgetManager::getInstance()->getInterfaceTimes();

			CGroupScrollText *pST = dynamic_cast<CGroupScrollText*>(pCaller->getParent());
			if (pST == NULL) return;
			if (pST->getList() == NULL) return;
			// get the font height from the text template of the list
			const CViewText *vt = pST->getList()->getTextTemplatePtr();
			if (!vt) return;
			pST->_EllapsedTime += times.frameDiffMs;
	//		pST->setH(std::max((sint32) pST->getMinHeight(), (sint32) (pST->_StartHeight - pST->_EllapsedTime / 9)));
			pST->setH((sint32) (pST->_StartHeight - pST->_EllapsedTime / 9));
			pST->invalidateCoords();
		}
	};
	REGISTER_ACTION_HANDLER (CSTDown, "gst_sub");

	// ***************************************************************************
	class CSTNewest : public IActionHandler
	{
	public:
		virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
		{
			CGroupScrollText *pST = dynamic_cast<CGroupScrollText*>(pCaller->getParent());
			if (pST) pST->scrollToNewest();
		}
	};
	REGISTER_ACTION_HANDLER (CSTNewest, "gst_newest");

	// ***************************************************************************
	class CSTUnread : public IActionHandler
	{
	public:
		virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
		{
			CGroupScrollText *pST = dynamic_cast<CGroupScrollText*>(pCaller->getParent());
			if (pST) pST->scrollToUnread();
		}
	};
	REGISTER_ACTION_HANDLER (CSTUnread, "gst_unread");

}

