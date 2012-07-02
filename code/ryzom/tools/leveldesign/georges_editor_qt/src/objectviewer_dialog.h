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

#ifndef OBJECTVIEWER_DIALOG_H
#define OBJECTVIEWER_DIALOG_H

#include <nel/misc/types_nl.h>
#include <nel/misc/event_emitter.h>

#include "ui_objectviewer_form.h"

// Qt includes
#include <QtOpenGL/QGLWidget>
#include <QtGui/QDockWidget>
#include <QTimer>

// STL includes

// NeL includes
#include <nel/misc/config_file.h>

// Project includes

#ifdef NL_OS_WINDOWS
//typedef QDockWidget QNLWidget;
typedef QWidget QNLWidget;
#elif defined(NL_OS_MAC)
typedef QWidget QNLWidget;
#else // NL_OS_UNIX
typedef QGLWidget QNLWidget;
#endif // NL_OS_UNIX

class QAction;

namespace NLQT 
{

	class CObjectViewerDialog: public QDockWidget
	{
		Q_OBJECT

	public:
		CObjectViewerDialog(QWidget *parent = 0);
		~CObjectViewerDialog();

		virtual void setVisible(bool visible);

		QAction *createSaveScreenshotAction(QObject *parent);
		QAction *createSetBackgroundColor(QObject *parent);

	private Q_SLOTS:
		void topLevelChanged(bool topLevel); 

	protected:
		virtual void resizeEvent(QResizeEvent *resizeEvent);

	private:
		CObjectViewerDialog(const CObjectViewerDialog &);
		CObjectViewerDialog &operator=(const CObjectViewerDialog &);

		Ui::CObjectViewerDialog _ui;
		QNLWidget * _nlw;

		friend class CMainWindow;
	}; /* CObjectViewerDialog */

} /* namespace NLQT */

#endif // OBJECTVIEWER_DIALOG_H
