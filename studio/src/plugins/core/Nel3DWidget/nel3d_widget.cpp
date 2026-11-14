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

#include "nel3d_widget.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/text_context.h"
#include "nel/3d/driver_user.h"
#include "nel/misc/rgba.h"
#include "nel/misc/path.h"

#ifdef NL_OS_WINDOWS
#include <Windows.h>
#endif

Nel3DWidget::Nel3DWidget(QWidget *parent)
    : Nel3DWidgetBase(parent)
{
	driver = NULL;
	textContext = NULL;

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
	driver->setDisplay((nlWindow)winId(), NL3D::UDriver::CMode(width(), height(), 32, true));
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

	Q_EMIT resize(evnt->size().width(), evnt->size().height());
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

