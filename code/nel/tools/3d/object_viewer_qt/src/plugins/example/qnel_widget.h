// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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

#ifndef QNEL_WIDGET_H
#define QNEL_WIDGET_H

// NeL includes
#include <nel/misc/types_nl.h>
#include <nel/misc/rgba.h>
#include <nel/misc/event_emitter.h>

// Qt includes
#include <QtOpenGL/QGLWidget>
#include <QtGui/QWidget>

class QAction;

/* TODO every platform should use QWidget */
#if defined(NL_OS_WINDOWS)
typedef QWidget QNeLWidget;
#elif defined(NL_OS_MAC)
typedef QWidget QNeLWidget;
#elif defined(NL_OS_UNIX)
typedef QGLWidget QNeLWidget;
#endif // NL_OS_UNIX

namespace NL3D
{
class UDriver;
class UScene;
}

namespace NLQT
{

/**
@class QNLWidget
@brief Responsible for interaction between Qt and NeL.
@details Automatically begins to update the render if the widget is visible
or suspends the updating of render if the widget is hidden.
*/
class QNLWidget : public QNeLWidget
{
	Q_OBJECT

public:
	QNLWidget(QWidget *parent);
	virtual ~QNLWidget();

	/// Set the update interval renderer
	void setInterval(int msec);

	/// Set the background color.
	void setBackgroundColor(NLMISC::CRGBA backgroundColor);

	float fps() const
	{
		return m_fps;
	}

	inline NLMISC::CRGBA backgroundColor() const
	{
		return m_backgroundColor;
	}

	NL3D::UDriver *driver() const
	{
		return m_driver;
	}

	virtual QPaintEngine *paintEngine() const
	{
		return NULL;
	}
Q_SIGNALS:
	void updateData();
	void updatePreRender();
	void updatePostRender();

private Q_SLOTS:
	void updateRender();

protected:
	virtual void showEvent(QShowEvent *showEvent);
	virtual void hideEvent(QHideEvent *hideEvent);

#if defined(NL_OS_WINDOWS)
	virtual bool winEvent(MSG *message, long *result);
#elif defined(NL_OS_MAC)
	virtual bool macEvent(EventHandlerCallRef caller, EventRef event);
#elif defined(NL_OS_UNIX)
	virtual bool x11Event(XEvent *event);
#endif

private:
	void init();
	void release();

	QNLWidget(const QNLWidget &);
	QNLWidget &operator=(const QNLWidget &);

	NL3D::UDriver *m_driver;
	NLMISC::CRGBA m_backgroundColor;

	QTimer *m_mainTimer;

	bool m_initialized;
	int m_interval;
	float m_fps;

}; /* class QNLWidget */

} /* namespace NLQT */


#endif // QNEL_WIDGET_H
