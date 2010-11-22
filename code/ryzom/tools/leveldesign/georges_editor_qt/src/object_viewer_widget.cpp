/*
Georges Editor Qt
Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "object_viewer_widget.h"

// STL includes

// NeL includes
#include <nel/3d/driver_user.h>

// Project includes
#include "modules.h"

namespace NLQT 
{

	CObjectViewerWidget::CObjectViewerWidget() 
	{

	}

	CObjectViewerWidget::~CObjectViewerWidget()
	{

	}

	void CObjectViewerWidget::setVisible(bool visible)
	{
		// called by show()
		// code assuming visible window needed to init the 3d driver
		nldebug("%d", visible);
		if (visible)
		{
			QWidget::setVisible(true);
		}
		else
		{
			QWidget::setVisible(false);
		}
	}

#if defined(NL_OS_WINDOWS)

	typedef bool (*winProc)(NL3D::IDriver *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	bool CObjectViewerWidget::winEvent(MSG * message, long * result)
	{
		if (Modules::objView().getDriver() && 
			Modules::objView().getDriver()->isActive())
		{
			NL3D::IDriver *driver = dynamic_cast<NL3D::CDriverUser*>(Modules::objView().getDriver())->getDriver();
			if (driver)
			{
				winProc proc = (winProc)driver->getWindowProc();
				return proc(driver, message->hwnd, message->message, message->wParam, message->lParam);
			}
		}

		return false;
	}

#elif defined(NL_OS_MAC)

	typedef bool (*cocoaProc)(NL3D::IDriver*, const void* e);

	bool CObjectViewerWidget::macEvent(EventHandlerCallRef caller, EventRef event)
	{
		if(caller)
			nlerror("You are using QtCarbon! Only QtCocoa supported, please upgrade Qt");

		if (Modules::objView().getDriver() && Modules::objView().getDriver()->isActive())
		{
			NL3D::IDriver *driver = dynamic_cast<NL3D::CDriverUser*>(Modules::objView().getDriver())->getDriver();
			if (driver)
			{
				cocoaProc proc = (cocoaProc)driver->getWindowProc();
				return proc(driver, event);
			}
		}

		return false;
	}

#elif defined(NL_OS_UNIX)

	typedef bool (*x11Proc)(NL3D::IDriver *drv, XEvent *e);

	bool CObjectViewerWidget::x11Event(XEvent *event)
	{
		if (Modules::objView().getDriver() && Modules::objView().getDriver()->isActive())
		{
			NL3D::IDriver *driver = dynamic_cast<NL3D::CDriverUser*>(Modules::objView().getDriver())->getDriver();
			if (driver)
			{
				x11Proc proc = (x11Proc)driver->getWindowProc();
				return proc(driver, event);
			}
		}

		return false;
	}
#endif

} /* namespace NLQT */
