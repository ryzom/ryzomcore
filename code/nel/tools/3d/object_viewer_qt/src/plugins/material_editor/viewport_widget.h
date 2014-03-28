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


#ifndef VIEWPORT_WIDGET_H
#define VIEWPORT_WIDGET_H

#include <QWidget>
#include "nel/misc/types_nl.h"
#include "nel/misc/event_emitter.h"

namespace MaterialEditor
{
	class CNel3DInterface;

	class ViewPortWidget : public QWidget
	{
		Q_OBJECT
	public:
		ViewPortWidget( QWidget *parent = NULL );
		~ViewPortWidget();
		void setNel3DInterface( CNel3DInterface *iface ){ nl3dIface = iface; }
		void init();

		void resizeEvent( QResizeEvent *evnt );

		QPaintEngine* paintEngine() const{ return NULL; }

		void startTimedUpdates( int interval );
		void stopTimedUpdates();

	protected:

		void timerEvent( QTimerEvent *evnt );

#if defined ( NL_OS_WINDOWS )
		bool winEvent( MSG *message, long *result );
#elif defined( NL_OS_MAC )
		bool macEvent( EventHandlerCallRef caller, EventRef event ) ;
#elif defined( NL_OS_UNIX )
		bool x11Event( XEvent *event );
#endif

	private:
		void update();

		CNel3DInterface *nl3dIface;
		int timerId;
	};
}


#endif


