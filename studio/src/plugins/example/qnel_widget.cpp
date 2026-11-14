// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2011  Dzmitry KAMIAHIN (dnk-88) <dnk-88@tut.by>
//
// This source file has been modified by the following contributors:
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

// Qt includes
#include <QtCore/QTimer>
#include <QtGui/QResizeEvent>

#include "qnel_widget.h"

// STL includes

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

// The driver does not subclass a window it did not create, so on Windows the messages of this
// window are passed on to it: mouse and size reach it, the keyboard does not, as Qt 5 keeps the
// Win32 keyboard focus on the top-level window, so key messages are sent there and not to this
// window. On X11 the driver gets no input from this window, since its event emitter asks for all
// input in one selection, which the X server refuses on a window whose button presses Qt already
// takes. On macOS nothing is passed on: Qt delivers NSEvent objects and the window proc of the
// driver takes a Carbon EventRef.
#if defined(NL_OS_WINDOWS)

typedef bool (*winProc)(NL3D::IDriver *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

bool QNLWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
	if (m_initialized && m_driver->isActive() && eventType == "windows_generic_MSG")
	{
		NL3D::IDriver *driver = dynamic_cast<NL3D::CDriverUser *>(m_driver)->getDriver();
		if (driver)
		{
			MSG *msg = static_cast<MSG *>(message);
			winProc proc = (winProc)driver->getWindowProc();
			if (proc(driver, msg->hwnd, msg->message, msg->wParam, msg->lParam))
			{
				*result = 0;
				return true;
			}
		}
	}

	return QNeLWidget::nativeEvent(eventType, message, result);
}

#endif

} /* namespace NLQT */

