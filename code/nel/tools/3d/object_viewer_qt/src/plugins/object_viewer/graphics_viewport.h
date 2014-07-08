/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef GRAPHICS_VIEWPORT_H
#define GRAPHICS_VIEWPORT_H

#include <nel/misc/types_nl.h>
#include <nel/misc/event_emitter.h>

// STL includes

// Qt includes
#include <QtOpenGL/QGLWidget>
#include <QtGui/QWidget>

// NeL includes

// Project includes

/* TODO every platform should use QWidget */
#if defined(NL_OS_WINDOWS)
typedef QWidget QNLWidget;
#elif defined(NL_OS_MAC)
typedef QWidget QNLWidget;
#elif defined(NL_OS_UNIX)
typedef QGLWidget QNLWidget;
#endif // NL_OS_UNIX

class QAction;

namespace NLQT
{

/**
@class CGraphicsViewport
@brief Responsible for interaction between Qt and NeL. Initializes CObjectViewer, CParticleEditor and CVegetableEditor subsystem.
*/
class CGraphicsViewport : public QNLWidget, public NLMISC::IEventEmitter
{
	Q_OBJECT

public:
	CGraphicsViewport(QWidget *parent);
	virtual ~CGraphicsViewport();

	virtual QPaintEngine *paintEngine() const
	{
		return NULL;
	}

	void init();
	void release();

	QAction *createSaveScreenshotAction(QObject *parent);
	QAction *createSetBackgroundColor(QObject *parent);

private Q_SLOTS:
	void saveScreenshot();
	void setBackgroundColor();

	void submitEvents(NLMISC::CEventServer &server, bool allWindows) { }

protected:
	virtual void resizeEvent(QResizeEvent *resizeEvent);

#if defined(NL_OS_WINDOWS)
	virtual bool winEvent(MSG *message, long *result);
#elif defined(NL_OS_MAC)
	virtual bool macEvent(EventHandlerCallRef caller, EventRef event);
#elif defined(NL_OS_UNIX)
	virtual bool x11Event(XEvent *event);
#endif

private:
	CGraphicsViewport(const CGraphicsViewport &);
	CGraphicsViewport &operator=(const CGraphicsViewport &);

}; /* class CGraphicsViewport */

} /* namespace NLQT */


#endif // GRAPHICS_VIEWPORT_H
