// Ryzom Core MMORPG framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012-2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

// moc does not see NL_OS_WINDOWS, so the base class is a typedef, which moc names as it is
#ifdef NL_OS_WINDOWS
#include <QWidget>
typedef QWidget Nel3DWidgetBase;
#else
#include <QGLWidget>
typedef QGLWidget Nel3DWidgetBase;
#endif

#include "../core_global.h"

namespace NL3D
{
	class UDriver;
	class UTextContext;
}

/// Nel 3D interface to Qt
class CORE_EXPORT Nel3DWidget : public Nel3DWidgetBase
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
	bool nativeEvent(const QByteArray &eventType, void *message, long *result) NL_OVERRIDE;
#endif

private:
	NL3D::UDriver *driver;
	NL3D::UTextContext *textContext;

};

#endif

