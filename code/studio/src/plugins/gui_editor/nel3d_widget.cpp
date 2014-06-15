// Object Viewer Qt GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#include "nel3d_widget.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/text_context.h"
#include "nel/3d/driver_user.h"
#include "nel/misc/rgba.h"
#include "nel/misc/path.h"
//#include "nel/misc/event_listener.h"

#ifdef NL_OS_WINDOWS
#include <Windows.h>
#endif



namespace GUIEditor
{
	Nel3DWidget::Nel3DWidget( QWidget *parent ) :
	QWidget( parent )
	{
		driver = NULL;
		textContext = NULL;

		// Need to set this attribute with a NULL paintengine returned to Qt
		// so that we can render the widget normally ourselves, without the image
		// disappearing when a widget is resized or shown on top of us
		setAttribute( Qt::WA_PaintOnScreen, true );
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
		driver->setMatrixMode2D11();
		driver->setDisplay( winId(), NL3D::UDriver::CMode( width(), height(), 32, true ) );
	}

	void Nel3DWidget::createTextContext( std::string fontFile )
	{
		if( driver == NULL )
			return;
		
		std::string font;

		try
		{
			font = NLMISC::CPath::lookup( fontFile );
		}
		catch( ... )
		{
			nlinfo( "Font %s cannot be found, cannot create textcontext!", fontFile.c_str() );
			exit( EXIT_FAILURE );
		}

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


#if defined ( NL_OS_WINDOWS )

	typedef bool ( *winProc )( NL3D::IDriver *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

	bool Nel3DWidget::winEvent( MSG *message, long *result )
	{
		if( driver != NULL ) 
		{
			NL3D::IDriver *iDriver = dynamic_cast< NL3D::CDriverUser* >( driver )->getDriver();
			if( iDriver != NULL )
			{
				winProc proc = (winProc)iDriver->getWindowProc();
				return proc( iDriver, message->hwnd, message->message, message->wParam, message->lParam );
			}
		}

		return false;
	}

#elif defined( NL_OS_MAC )

	typedef bool ( *cocoaProc )( NL3D::IDriver *, const void *e );

	bool Nel3DWidget::macEvent( EventHandlerCallRef caller, EventRef event )
	{
		if( caller )
			nlerror( "You are using QtCarbon! Only QtCocoa supported, please upgrade Qt" );

		if( driver != NULL )
		{
			NL3D::IDriver *iDriver = dynamic_cast< NL3D::CDriverUser* >( driver )->getDriver();
			if( iDriver != NULL )
			{
				cocoaProc proc = ( cocoaProc )iDriver->getWindowProc();
				return proc( iDriver, event );
			}
		}

		return false;
	}

#elif defined( NL_OS_UNIX )

	typedef bool ( *x11Proc )( NL3D::IDriver *drv, XEvent *e );

	bool Nel3DWidget::x11Event( XEvent *event )
	{
		if( driver != NULL )
		{
			NL3D::IDriver *iDriver = dynamic_cast< NL3D::CDriverUser* >( driver )->getDriver();
			if( driver != NULL )
			{
				x11Proc proc = ( x11Proc )iDriver->getWindowProc();
				return proc( iDriver, event );
			}
		}

		return false;
	}
#endif 

}


