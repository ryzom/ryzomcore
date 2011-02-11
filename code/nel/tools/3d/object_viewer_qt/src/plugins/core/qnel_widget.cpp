// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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

#include "qnel_widget.h"

// STL includes

// Qt includes
#include <QtCore/QTimer>
#include <QtGui/QResizeEvent>

// NeL includes
#include <nel/misc/event_server.h>
#include <nel/misc/debug.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/driver_user.h>

namespace NLQT
{

QNLWidget::QNLWidget(QWidget *parent)
	: QWidget(parent),
	  _driver(NULL),
	  _initialized(false),
	  _interval(25)
{
	init();
	_mainTimer = new QTimer(this);
	connect(_mainTimer, SIGNAL(timeout()), this, SLOT(updateRender()));
}

QNLWidget::~QNLWidget()
{
	release();
}

void QNLWidget::init()
{
	// create the driver
	_driver = NL3D::UDriver::createDriver(NULL, false, NULL);
	nlassert(_driver);

	// initialize the nel 3d viewport
	_driver->setDisplay((nlWindow)winId(), NL3D::UDriver::CMode(width(), height(), 32));

	// set the cache size for the font manager(in bytes)
	_driver->setFontManagerMaxMemory(2097152);

	_initialized = true;
}

void QNLWidget::release()
{
	_mainTimer->stop();
	delete _mainTimer;
	if (_initialized)
	{
		_driver->release();
		delete _driver;
		_driver = NULL;
	}
}

void QNLWidget::setInterval(int msec)
{
	_interval = msec;
	_mainTimer->setInterval(msec);
}

void QNLWidget::updateRender()
{
	if (isVisible())
	{
		if (_initialized)
			_driver->EventServer.pump();
		if (_initialized && !_driver->isLost())
		{
			_driver->activate();
			_driver->clearBuffers(NLMISC::CRGBA(125,12,58));

			// swap 3d buffers
			_driver->swapBuffers();
		}
	}
}

void QNLWidget::showEvent(QShowEvent *showEvent)
{
	QWidget::showEvent(showEvent);
	if (isVisible())
	{
		_driver->activate();
		_mainTimer->start(_interval);
	}
	else
		_mainTimer->stop();
}

#if defined(NL_OS_WINDOWS) || defined(NL_OS_MAC)
// Qt does not provide wheel events through winEvent() and macEvent() (but it
// does through x11Event(), which is inconsistent...)
// Workaround is to handle wheel events like implemented below.
//
// TODO: this is not a clean solution, because all but wheel events are
// handled using winEvent(), x11Event(), macEvent(). But this seems to be a
// limitation of current (4.7.1) Qt versions. (see e.g. qapplication_mac.mm)
void QNLWidget::wheelEvent(QWheelEvent *event)
{
	// Get relative positions.
	float fX = 1.0f - (float)event->pos().x() / this->width();
	float fY = 1.0f - (float)event->pos().y() / this->height();

	// Get the buttons currently pressed.
	uint32 buttons = NLMISC::noButton;
	if(event->buttons() & Qt::LeftButton)        buttons |= NLMISC::leftButton;
	if(event->buttons() & Qt::RightButton)       buttons |= NLMISC::rightButton;
	if(event->buttons() & Qt::MidButton)         buttons |= NLMISC::middleButton;
	if(event->modifiers() & Qt::ControlModifier) buttons |= NLMISC::ctrlButton;
	if(event->modifiers() & Qt::ShiftModifier)   buttons |= NLMISC::shiftButton;
	if(event->modifiers() & Qt::AltModifier)     buttons |= NLMISC::altButton;

	if(event->delta() > 0)
		_driver->EventServer.postEvent(
			new NLMISC::CEventMouseWheel(-fX, fY, (NLMISC::TMouseButton)buttons, true, NULL));
	else
		_driver->EventServer.postEvent(
			new NLMISC::CEventMouseWheel(-fX, fY, (NLMISC::TMouseButton)buttons, false, NULL));
}
#endif // defined(NL_OS_WINDOWS) || defined(NL_OS_MAC)


#if defined(NL_OS_WINDOWS)

typedef bool (*winProc)(NL3D::IDriver *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

bool QNLWidget::winEvent(MSG *message, long *result)
{
	if (_driver && _driver->isActive())
	{
		NL3D::IDriver *driver = dynamic_cast<NL3D::CDriverUser *>(_driver)->getDriver();
		if (driver)
		{
			winProc proc = (winProc)driver->getWindowProc();
			return proc(driver, message->hwnd, message->message, message->wParam, message->lParam);
		}
	}

	return false;
}

#elif defined(NL_OS_MAC)

typedef bool (*cocoaProc)(NL3D::IDriver *, const void *e);

bool QNLWidget::macEvent(EventHandlerCallRef caller, EventRef event)
{
	if(caller)
		nlerror("You are using QtCarbon! Only QtCocoa supported, please upgrade Qt");

	if (_driver && _driver->isActive())
	{
		NL3D::IDriver *driver = dynamic_cast<NL3D::CDriverUser *>(_driver)->getDriver();
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

bool QNLWidget::x11Event(XEvent *event)
{
	if (_driver && _driver->isActive())
	{
		NL3D::IDriver *driver = dynamic_cast<NL3D::CDriverUser *>(_driver)->getDriver();
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

