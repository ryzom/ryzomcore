// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2011  Dzmitry KAMIAHIN (dnk-88) <dnk-88@tut.by>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
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
	m_driver = NL3D::UDriver::createDriver(NULL, false, NULL);
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
#elif defined(NL_OS_MAC)
typedef bool (*cocoaProc)(NL3D::IDriver *, const void *e);
#endif

bool QNLWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
	if (!m_driver || !m_driver->isActive())
		return QWidget::nativeEvent(eventType, message, result);

	NL3D::IDriver *driver = dynamic_cast<NL3D::CDriverUser *>(m_driver)->getDriver();
	if (!driver)
		return QWidget::nativeEvent(eventType, message, result);

#if defined(NL_OS_WINDOWS)
	if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
	{
		MSG *msg = static_cast<MSG *>(message);
		winProc proc = (winProc)driver->getWindowProc();
		return proc(driver, msg->hwnd, msg->message, msg->wParam, msg->lParam);
	}
#elif defined(NL_OS_MAC)
	if (eventType == "NSEvent")
	{
		cocoaProc proc = (cocoaProc)driver->getWindowProc();
		return proc(driver, message);
	}
#elif defined(NL_OS_UNIX)
	// Through the xcb platform Qt5 delivers an xcb_generic_event_t, while NeL's window
	// procedure (unix_event_emitter) expects an Xlib XEvent structure. Not convertible
	// into one another, so nothing is forwarded here on purpose - input goes through the
	// ordinary Qt events.
#endif

	return QWidget::nativeEvent(eventType, message, result);
}


} /* namespace NLQT */

