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
	: QNeLWidget(parent),
	  m_driver(NULL),
	  m_initialized(false),
	  m_interval(25)
{
	setAttribute(Qt::WA_OpaquePaintEvent);
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_PaintOnScreen);
	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);

	init();
#ifdef Q_OS_LINUX
	makeCurrent();
#endif
	m_mainTimer = new QTimer(this);
	connect(m_mainTimer, SIGNAL(timeout()), this, SLOT(updateRender()));
}

QNLWidget::~QNLWidget()
{
	release();
}

void QNLWidget::init()
{
	// create the driver
	m_driver = NL3D::UDriver::createDriver(0, false);
	nlassert(m_driver);

	// initialize the nel 3d viewport
	m_driver->setDisplay((nlWindow)winId(), NL3D::UDriver::CMode(width(), height(), 32));

	// set the cache size for the font manager(in bytes)
	m_driver->setFontManagerMaxMemory(2097152);

	m_initialized = true;
}

void QNLWidget::release()
{
	m_mainTimer->stop();
	delete m_mainTimer;
	if (m_initialized)
	{
		m_driver->release();
		delete m_driver;
		m_driver = NULL;
	}
}

void QNLWidget::setInterval(int msec)
{
	m_interval = msec;
	m_mainTimer->setInterval(msec);
}

void QNLWidget::setBackgroundColor(NLMISC::CRGBA backgroundColor)
{
	m_backgroundColor = backgroundColor;
}

void QNLWidget::updateRender()
{
	if (isVisible())
	{
		if (m_initialized)
			m_driver->EventServer.pump();
		Q_EMIT updateData();

		// Calc FPS
		static sint64 lastTime = NLMISC::CTime::getPerformanceTime ();
		sint64 newTime = NLMISC::CTime::getPerformanceTime ();
		m_fps = float(1.0 / NLMISC::CTime::ticksToSecond (newTime-lastTime));
		lastTime = newTime;

		if (m_initialized && !m_driver->isLost())
		{
			//_driver->activate();
			m_driver->clearBuffers(m_backgroundColor);
			Q_EMIT updatePreRender();

			Q_EMIT updatePostRender();
			// swap 3d buffers
			m_driver->swapBuffers();
		}
	}
}

void QNLWidget::showEvent(QShowEvent *showEvent)
{
	QWidget::showEvent(showEvent);
	m_driver->activate();
	m_mainTimer->start(m_interval);
}

void QNLWidget::hideEvent(QHideEvent *hideEvent)
{
	m_mainTimer->stop();
	QWidget::hideEvent(hideEvent);
}

#if defined(NL_OS_WINDOWS)

typedef bool (*winProc)(NL3D::IDriver *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

bool QNLWidget::winEvent(MSG *message, long *result)
{
	if (m_driver && m_driver->isActive())
	{
		NL3D::IDriver *driver = dynamic_cast<NL3D::CDriverUser *>(m_driver)->getDriver();
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

	if (m_driver && m_driver->isActive())
	{
		NL3D::IDriver *driver = dynamic_cast<NL3D::CDriverUser *>(m_driver)->getDriver();
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
	if (m_driver && m_driver->isActive())
	{
		NL3D::IDriver *driver = dynamic_cast<NL3D::CDriverUser *>(m_driver)->getDriver();
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

