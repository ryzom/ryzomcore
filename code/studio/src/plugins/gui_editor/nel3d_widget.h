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


#ifndef NEL3D_WIDGET_H
#define NEL3D_WIDGET_H

#include <QWidget>
#include "nel/misc/types_nl.h"
#include <string>

namespace NL3D
{
	class UDriver;
	class UTextContext;
}

namespace GUIEditor
{
	/// Nel 3D interface to Qt
	class Nel3DWidget : public QWidget
	{
		Q_OBJECT
	public:
		Nel3DWidget( QWidget *parent = NULL );
		virtual ~Nel3DWidget();

		virtual void init();
		void createTextContext( std::string fontFile );

		NL3D::UDriver* getDriver() const{ return driver; }
		NL3D::UTextContext* getTextContext() const{ return textContext; }


		// Need to return NULL paintengine to Qt so that we can
		// render the widget normally ourselves, without the image
		// disappearing when a widget is resized or shown on top of us
		QPaintEngine* paintEngine() const{ return NULL; }

	public Q_SLOTS:
		void clear();

	protected:

#if defined(NL_OS_WINDOWS)
	virtual bool winEvent( MSG *message, long *result );
#elif defined(NL_OS_MAC)
	virtual bool macEvent( EventHandlerCallRef caller, EventRef event );
#elif defined(NL_OS_UNIX)
	virtual bool x11Event( XEvent *event );
#endif

	private:
		NL3D::UDriver *driver;
		NL3D::UTextContext *textContext;
	};
}

#endif

