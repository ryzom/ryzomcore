// Ryzom Core MMORPG framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013-2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

// needs to be included before x11 due to some collisions
#include <QResizeEvent>
#include <QSurfaceFormat>
#include <QWindow>

#include "nel3d_widget.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/text_context.h"
#include "nel/3d/driver_user.h"
#include "nel/misc/rgba.h"
#include "nel/misc/path.h"

#ifdef NL_OS_WINDOWS
#include <Windows.h>
#endif

// The mode of the driver for a size of the widget: on X11 the driver takes the size of the window
// from the mode, and the native window is in device pixels
static NL3D::UDriver::CMode driverMode(const QWidget *widget, const QSize &size)
{
#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
	const qreal ratio = widget->devicePixelRatioF();
	return NL3D::UDriver::CMode((uint16)qRound(size.width() * ratio), (uint16)qRound(size.height() * ratio), 32, true);
#else
	nlunreferenced(widget);
	return NL3D::UDriver::CMode((uint16)size.width(), (uint16)size.height(), 32, true);
#endif
}

Nel3DWidget::Nel3DWidget(QWidget *parent)
    : QWidget(parent)
{
	driver = NULL;
	textContext = NULL;

	// The driver renders into the native window of this widget with its own context
	setAttribute(Qt::WA_NativeWindow, true);

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
	// on X11 the driver renders with the visual of this window, so a depth and stencil buffer
	// are asked for before the window is created
	if (QWindow *window = windowHandle())
	{
		QSurfaceFormat format = window->requestedFormat();
		format.setDepthBufferSize(24);
		format.setStencilBufferSize(8);
		window->setFormat(format);
	}
#endif

	// Need to set this attribute with a NULL paintengine returned to Qt
	// so that we can render the widget normally ourselves, without the image
	// disappearing when a widget is resized or shown on top of us
	setAttribute( Qt::WA_PaintOnScreen, true );
	setAttribute( Qt::WA_OpaquePaintEvent, true );
	setAttribute( Qt::WA_NoSystemBackground, true );
}

Nel3DWidget::~Nel3DWidget()
{
	if( driver != NULL )
	{
		if( textContext != NULL )
		{
			driver->deleteTextContext( textContext );
			textContext = NULL;
		}

		driver->release();
		delete driver;
		driver = NULL;
	}
}

void Nel3DWidget::init()
{
	nlassert( driver == NULL );

	driver = NL3D::UDriver::createDriver( 0, false, 0 );
	if (!driver->setDisplay((nlWindow)winId(), driverMode(this, size())))
		throw NLMISC::Exception(std::string("The 3D driver cannot render into the window of the NeL 3D widget"));
}

void Nel3DWidget::createTextContext( std::string fontFile )
{
	if( driver == NULL )
		return;
		
	std::string font = NLMISC::CPath::lookup(fontFile, false);
	if (font.empty())
		throw NLMISC::Exception(std::string("The font '") + fontFile + "' is not in the search paths");

	if( textContext != NULL )
	{
		driver->deleteTextContext( textContext );
		textContext = NULL;
	}

	textContext = driver->createTextContext( font );
}

void Nel3DWidget::clear()
{
	if( driver == NULL )
		return;
	driver->clearBuffers( NLMISC::CRGBA::Black );
	driver->swapBuffers();
}

void Nel3DWidget::showEvent( QShowEvent *evnt )
{
	QWidget::showEvent( evnt );

	if( driver != NULL )
		driver->activate();
}

void Nel3DWidget::resizeEvent( QResizeEvent *evnt )
{
	QWidget::resizeEvent( evnt );

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
	// on X11 the driver gets no events of this window, it takes its size from setMode
	if (driver != NULL && evnt->size().width() > 0 && evnt->size().height() > 0)
		driver->setMode(driverMode(this, evnt->size()));
#endif

	Q_EMIT resize(evnt->size().width(), evnt->size().height());
}

// The driver does not subclass a window it did not create, so on Windows the messages of this
// window are passed on to it: mouse and size reach it, the keyboard does not, as Qt 5 keeps the
// Win32 keyboard focus on the top-level window, so key messages are sent there and not to this
// window. On X11 the driver draws at the size given through setMode but gets no input, since its
// event emitter asks for all input in one selection, which the X server refuses on a window whose
// button presses Qt already takes. On macOS nothing is passed on: Qt delivers NSEvent objects and
// the window proc of the driver takes a Carbon EventRef.
#if defined(NL_OS_WINDOWS)

typedef bool (*winProc)(NL3D::IDriver *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

bool Nel3DWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
	if (driver != NULL && eventType == "windows_generic_MSG")
	{
		NL3D::IDriver *iDriver = dynamic_cast<NL3D::CDriverUser *>(driver)->getDriver();
		if (iDriver != NULL)
		{
			MSG *msg = static_cast<MSG *>(message);
			winProc proc = (winProc)iDriver->getWindowProc();
			if (proc(iDriver, msg->hwnd, msg->message, msg->wParam, msg->lParam))
			{
				*result = 0;
				return true;
			}
		}
	}

	return QWidget::nativeEvent(eventType, message, result);
}

#endif

