// Ryzom Core MMORPG framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "nel/misc/types_nl.h"
#include <string>

#ifdef NEL3DWIDGET
#undef NEL3DWIDGET
#endif

#ifdef NL_OS_WINDOWS
#include <QWidget>
#define NEL3DWIDGET QWidget
#else
#include <QGLWidget>
#define NEL3DWIDGET QGLWidget
#endif


#include "../core_global.h"

namespace NL3D
{
	class UDriver;
	class UTextContext;
}

/// Nel 3D interface to Qt
class CORE_EXPORT Nel3DWidget : public NEL3DWIDGET
{
	Q_OBJECT
public:
	Nel3DWidget( QWidget *parent = NULL );
	virtual ~Nel3DWidget();

	void init();
	void createTextContext( std::string fontFile );

	NL3D::UDriver* getDriver() const{ return driver; }
	NL3D::UTextContext* getTextContext() const{ return textContext; }


	// Need to return NULL paintengine to Qt so that we can
	// render the widget normally ourselves, without the image
	// disappearing when a widget is resized or shown on top of us
	QPaintEngine* paintEngine() const{ return NULL; }

Q_SIGNALS:
	void resize( int width, int height );

public Q_SLOTS:
	void clear();

protected:

	void showEvent( QShowEvent *evnt );
	void resizeEvent( QResizeEvent *evnt );

#if defined(NL_OS_WINDOWS)
	bool winEvent( MSG *message, long *result );
#elif defined(NL_OS_MAC)
	bool macEvent( EventHandlerCallRef caller, EventRef event );
#elif defined(NL_OS_UNIX)
	bool x11Event( XEvent *event );
#endif

private:
	NL3D::UDriver *driver;
	NL3D::UTextContext *textContext;

};

#endif

