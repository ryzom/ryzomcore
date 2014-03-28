// Object Viewer Qt Material Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#include "viewport_widget.h"
#include <QResizeEvent>
#include "nel3d_interface.h"
#include "nel/3d/driver.h"
#include "nel/3d/driver_user.h"

#ifdef NL_OS_WINDOWS
#include <Windows.h>
#endif


namespace MaterialEditor
{
	ViewPortWidget::ViewPortWidget( QWidget *parent ) :
	QWidget( parent )
	{
		nl3dIface = NULL;
		timerId = 0;
		setAttribute( Qt::WA_PaintOnScreen );
	}

	ViewPortWidget::~ViewPortWidget()
	{
	}

	void ViewPortWidget::init()
	{
		nl3dIface->initViewPort( (unsigned long)winId(), width(), height() );
		
	}

	void ViewPortWidget::resizeEvent( QResizeEvent *evnt )
	{
		uint32 w = evnt->size().width();
		uint32 h = evnt->size().height();

		nl3dIface->resizeViewPort( w, h );

		QWidget::resizeEvent( evnt );
	}

	void ViewPortWidget::startTimedUpdates( int interval )
	{
		if( interval == 0 )
			return;

		timerId = startTimer( interval );
	}

	void ViewPortWidget::stopTimedUpdates()
	{
		killTimer( timerId );
		timerId = 0;
	}

	void ViewPortWidget::timerEvent( QTimerEvent *evnt )
	{
		int id = evnt->timerId();
		if( id == timerId )
			update();
	}

#if defined( NL_OS_WINDOWS )
	
	typedef bool ( *winProc )( NL3D::IDriver *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
	
	bool ViewPortWidget::winEvent( MSG *message, long *result )
	{
		NL3D::UDriver *udriver = nl3dIface->getDriver();
		if( ( udriver != NULL ) && udriver->isActive() )
		{
			NL3D::IDriver *driver = dynamic_cast< NL3D::CDriverUser* >( udriver )->getDriver();
			if( driver != NULL )
			{
				winProc proc = (winProc)driver->getWindowProc();
				return proc( driver, message->hwnd, message->message, message->wParam, message->lParam );
			}
		}
		
		return false;
	}

#elif defined( NL_OS_MAC )

	typedef bool ( *cocoaProc )( NL3D::IDriver *, const void *e );
	
	bool ViewPortWidget::macEvent( EventHandlerCallRef caller, EventRef event )
	{
		if( caller )
			nlerror("You are using QtCarbon! Only QtCocoa supported, please upgrade Qt");
		
		NL3D::UDriver *udriver = nl3dIface->getDriver();
		if( ( udriver != NULL ) && udriver->isActive() )
		{
			NL3D::IDriver *driver = dynamic_cast< NL3D::CDriverUser* >( udriver )->getDriver();
			if( driver != NULL )
			{
				cocoaProc proc = ( cocoaProc )driver->getWindowProc();
				proc( driver, event );
			}
		}
		
		return false;
	}

#elif defined( NL_OS_UNIX )
	
	typedef bool ( *x11Proc )( NL3D::IDriver *drv, XEvent *e );
	
	bool ViewPortWidget::x11Event( XEvent *event )
	{
		NL3D::UDriver *udriver = nl3dIface->getDriver();
		
		if( ( udriver != NULL ) && udriver->isActive() )
		{
			NL3D::IDriver *driver = dynamic_cast< NL3D::CDriverUser* >( udriver )->getDriver();
			if( driver != NULL )
			{
				x11Proc proc = ( x11Proc )driver->getWindowProc();
				proc( driver, event );
			}
		
		}
		return false;
	}

#endif

	void ViewPortWidget::update()
	{
		nl3dIface->updateInput();
		nl3dIface->renderScene();
	}

}


