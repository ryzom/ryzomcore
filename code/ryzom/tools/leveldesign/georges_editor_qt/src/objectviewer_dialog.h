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
#else // NL_OS_UNIX
typedef QGLWidget QNLWidget;
#endif // NL_OS_UNIX

class QAction;

namespace NLQT {
  
class CObjectViewerDialog: public QDockWidget, public NLMISC::IEventEmitter
{
     Q_OBJECT

public:
	CObjectViewerDialog(QWidget *parent = 0);
	~CObjectViewerDialog();

	virtual void setVisible(bool visible);
	// virtual QPaintEngine* paintEngine() const { return NULL; }

	void init();
	void release();
	void reinit();

	QAction *createSaveScreenshotAction(QObject *parent);
	QAction *createSetBackgroundColor(QObject *parent);

private Q_SLOTS:
	void updateRender();

	void saveScreenshot();
	void setBackgroundColor();

	void submitEvents(NLMISC::CEventServer &server, bool allWindows) { };
	void emulateMouseRawMode(bool) { };

	void topLevelChanged(bool topLevel); 

protected:
	virtual void resizeEvent(QResizeEvent *resizeEvent);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	
	uint32 getNelButtons(QMouseEvent *event);
	uint32 getNelButtons(QWheelEvent *event);
	
private:
	CObjectViewerDialog(const CObjectViewerDialog &);
	CObjectViewerDialog &operator=(const CObjectViewerDialog &);

	void updateInitialization(bool visible);

	Ui::CObjectViewerDialog _ui;
	
	// render stuff
	QTimer *_mainTimer;
	bool _isGraphicsInitialized, _isGraphicsEnabled;

	QNLWidget * _nlw;

	friend class CMainWindow;
}; /* CObjectViewerDialog */

} /* namespace NLQT */

#endif // OBJECTVIEWER_DIALOG_H
