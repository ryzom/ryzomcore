/*
Georges Editor Qt
Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

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

#ifndef OBJECT_VIEWER_WIDGET_H
#define OBJECT_VIEWER_WIDGET_H

// STL includes

// NeL includes

// Qt includes
#include <QtOpenGL/QGLWidget>

// Project includes

/**
namespace NLQT
@brief namespace NLQT
*/
namespace NLQT 
{
	class CObjectViewerWidget: public QWidget
	{
		Q_OBJECT

	public:
		/// Default constructor.
		CObjectViewerWidget();
		virtual ~CObjectViewerWidget();

		virtual void setVisible(bool visible);

	protected:
#if defined(NL_OS_WINDOWS)
		virtual bool winEvent(MSG * message, long * result);
#elif defined(NL_OS_MAC)
		virtual bool macEvent(EventHandlerCallRef caller, EventRef event);
#elif defined(NL_OS_UNIX)
		virtual bool x11Event(XEvent *event);
#endif

	};/* class CObjectViewerWidget */

} /* namespace NLQT */

#endif // OBJECT_VIEWER_WIDGET_H
