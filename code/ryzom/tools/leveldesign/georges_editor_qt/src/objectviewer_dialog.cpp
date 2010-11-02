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

#include "objectviewer_dialog.h"

// Qt includes
#include <QtGui/QAction>
#include <QtGui/QResizeEvent>
#include <QtGui/QColorDialog>
#include <QtGui/QFileDialog>
#include <QLabel>

// NeL includes
#include <nel/misc/event_server.h>
#include <nel/misc/events.h>

#include <nel/3d/u_driver.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_scene.h>

// Project includes
#include "modules.h"

using namespace std;
using namespace NL3D;

namespace NLQT {
  
CObjectViewerDialog::CObjectViewerDialog(QWidget *parent)
: _isGraphicsInitialized(false), _isGraphicsEnabled(false), QDockWidget(parent)
{
	_ui.setupUi(this);

	//widget = new QWidget(dockWidgetContents);
	//widget->setObjectName(QString::fromUtf8("widget"));
	

	_nlw = new QNLWidget(_ui.dockWidgetContents);
	_nlw->setObjectName(QString::fromUtf8("nlwidget"));
	_ui.gridLayout->addWidget(_nlw, 0, 0, 1, 1);
	//nlw->setLayout(new QGridLayout(nlw));
	//_ui.widget = nlw;
	//QWidget * w = widget();

	_isGraphicsEnabled = true;

	// As a special case, a QTimer with a timeout of 0 will time out as soon as all the events in the window system's event queue have been processed. 
	// This can be used to do heavy work while providing a snappy user interface.
	_mainTimer = new QTimer(this);
	connect(_mainTimer, SIGNAL(timeout()), this, SLOT(updateRender()));
	// timer->start(); // <- timeout 0
	// it's heavy on cpu, though, when no 3d driver initialized :)
	_mainTimer->start(5); // 25fps
}

CObjectViewerDialog::~CObjectViewerDialog() {
	_mainTimer->stop();
}

void CObjectViewerDialog::init()
{
	connect(this, SIGNAL(topLevelChanged(bool)),
		this, SLOT(topLevelChanged(bool)));
	//H_AUTO2
	nldebug("CObjectViewerDialog::init %d",_nlw->winId());
	
#ifdef NL_OS_UNIX
	dynamic_cast<QNLWidget*>(widget())->makeCurrent();
#endif // NL_OS_UNIX
	
	Modules::objView().init((nlWindow)_nlw->winId(), 20, 20);
	setMouseTracking(true);
}

void CObjectViewerDialog::setVisible(bool visible)
{
	// called by show()
	// code assuming visible window needed to init the 3d driver
	if (visible != isVisible())
	{
		if (visible)
		{
			QDockWidget::setVisible(true);
			updateInitialization(true);
		}
		else
		{
			updateInitialization(false);
			QDockWidget::setVisible(false);
		}
	}
}

void CObjectViewerDialog::updateInitialization(bool visible)
{
	//nldebug("CMainWindow::updateInitialization");
	bool done;
	do
	{
		done = true; // set false whenever change
		bool wantGraphics = _isGraphicsEnabled && visible;
		// bool wantLandscape = wantGraphics && m_IsGraphicsInitialized && isLandscapeEnabled;

		// .. stuff that depends on other stuff goes on top to prioritize deinitialization

		// Landscape
		// ...

		// Graphics (Driver)
		if (_isGraphicsInitialized)
		{
			if (!wantGraphics)
			{
				_isGraphicsInitialized = false;
				release();
				_mainTimer->stop();
				done = false;
			}
	
		}
		else
		{
			if (wantGraphics)
			{
				init();
				_isGraphicsInitialized = true;
				_mainTimer->start(5);
				done = false;
			}
		}
		


	} while (!done);
}

void CObjectViewerDialog::updateRender()
{
	//nldebug("CMainWindow::updateRender");
	updateInitialization(isVisible());
	
	//QModelIndex index = _dirModel->setRootPath("D:/Dev/Ryzom/code/ryzom/common/data_leveldesign/leveldesign");
	//_dirTree->setRootIndex(index);

	if (isVisible())
	{
		// call all update functions
		// 01. Update Utilities (configuration etc)
		
		// 02. Update Time (deltas)
		// ...

		// 03. Update Receive (network, servertime, receive messages)
		// ...
		
		// 04. Update Input (keyboard controls, etc)
		if (_isGraphicsInitialized)
			Modules::objView().updateInput();
		
		// 05. Update Weather (sky, snow, wind, fog, sun)
		// ...

		// 06. Update Entities (movement, do after possible tp from incoming messages etc)
		//      - Move other entities
		//      - Update self entity
		//      - Move bullets
		// ...

		// 07. Update Landscape (async zone loading near entity)
		// ...
		
		// 08. Update Collisions (entities)
		//      - Update entities
		//      - Update move container (swap with Update entities? todo: check code!)
		//      - Update bullets
		// ...
		
		// 09. Update Animations (playlists)
		//      - Needs to be either before or after entities, not sure, 
		//        there was a problem with wrong order a while ago!!!

		
		//Modules::objView().updateAnimation(_AnimationDialog->getTime());
		
		// 10. Update Camera (depends on entities)
		// ...
		
		// 11. Update Interface (login, ui, etc)
		// ...
		
		// 12. Update Sound (sound driver)
		// ...
		
		// 13. Update Send (network, send new position etc)
		// ...
		
		// 14. Update Debug (stuff for dev)
		// ...
		
		if (_isGraphicsInitialized && !Modules::objView().getDriver()->isLost())
		{
			// 01. Render Driver (background color)			
			Modules::objView().renderDriver(); // clear all buffers
				
			// 02. Render Sky (sky scene)
			// ...
			
			// 04. Render Scene (entity scene)
			Modules::objView().renderScene();
			
			// 05. Render Effects (flare)
			// ...
			
			// 06. Render Interface 3D (player names)
			// ...

			// 07. Render Debug 3D
			// ...

			// 08. Render Interface 2D (chatboxes etc, optionally does have 3d)
			// ...
				
			// 09. Render Debug 2D (stuff for dev)
			Modules::objView().renderDebug2D();

			// swap 3d buffers
			Modules::objView().getDriver()->swapBuffers();
		}
	}
}

void CObjectViewerDialog::release()
{
	//H_AUTO2
	nldebug("CObjectViewerDialog::release");
	
	Modules::objView().release();
}

void CObjectViewerDialog::reinit()
{
	//H_AUTO2
	nldebug("CObjectViewerDialog::reinit");
	
	Modules::objView().release();
	//Modules::objView().reinit(_ui.frame->winId(), width(), height());
}

QAction *CObjectViewerDialog::createSaveScreenshotAction(QObject *parent)
{
	QAction *action = new QAction(parent);
	connect(action, SIGNAL(triggered()), this, SLOT(saveScreenshot()));
	return action;
}

QAction *CObjectViewerDialog::createSetBackgroundColor(QObject *parent)
{
  	QAction *action = new QAction(parent);
	connect(action, SIGNAL(triggered()), this, SLOT(setBackgroundColor()));
	return action;
}

void CObjectViewerDialog::saveScreenshot()
{
	Modules::objView().saveScreenshot("screenshot", false, true, false);
}

void CObjectViewerDialog::setBackgroundColor()
{
	QColor color = QColorDialog::getColor(QColor(Modules::objView().getBackgroundColor().R,
						     Modules::objView().getBackgroundColor().G,
						     Modules::objView().getBackgroundColor().B));
	Modules::objView().setBackgroundColor(NLMISC::CRGBA(color.red(), color.green(), color.blue()));
}

void CObjectViewerDialog::topLevelChanged ( bool topLevel ) {
	//nldebug("CObjectViewerDialog::topLevelChanged topLevel:%d",topLevel);
	nldebug("CObjectViewerDialog::topLevelChanged winId:%d",winId());
	// winId changing when re/docking
	//Modules::georges().init();
	Modules::objView().reinit((nlWindow)_nlw->winId(), _nlw->width(), _nlw->height());
}

void CObjectViewerDialog::resizeEvent(QResizeEvent *resizeEvent)
{
	QDockWidget::resizeEvent(resizeEvent);
	if (Modules::objView().getDriver())
		Modules::objView().setSizeViewport(resizeEvent->size().width(), resizeEvent->size().height());

	// The OpenGL driver does not resize automatically.
	// The Direct3D driver breaks the window mode to include window borders when calling setMode windowed.

	// Resizing the window after switching drivers a few times becomes slow.
	// There is probably something inside the drivers not being released properly.
}

void CObjectViewerDialog::wheelEvent(QWheelEvent *event)
{
	//nldebug("CObjectViewerDialog::wheelEvent");
	// Get relative positions.
	float fX = 1.0f - (float)event->pos().x() / this->width();
	float fY = 1.0f - (float)event->pos().y() / this->height();
	
	// Set the buttons currently pressed.
	NLMISC::TMouseButton buttons = (NLMISC::TMouseButton)getNelButtons(event);
	if(event->delta() > 0)
		Modules::objView().getDriver()->EventServer.postEvent(new NLMISC::CEventMouseWheel(-fX, fY, buttons, true, this));
	else
		Modules::objView().getDriver()->EventServer.postEvent(new NLMISC::CEventMouseWheel(-fX, fY, buttons, false, this));
	
}

uint32  CObjectViewerDialog::getNelButtons(QMouseEvent *event) {
	//nldebug("CObjectViewerDialog::getNelButtons");
	uint32 buttons = NLMISC::noButton;
	if(event->buttons() & Qt::LeftButton)	buttons |= NLMISC::leftButton;
	if(event->buttons() & Qt::RightButton)	buttons |= NLMISC::rightButton;
	if(event->buttons() & Qt::MidButton)	buttons |= NLMISC::middleButton;
	if(event->modifiers() & Qt::ControlModifier)	buttons |= NLMISC::ctrlButton;
	if(event->modifiers() & Qt::ShiftModifier)	buttons |= NLMISC::shiftButton;
	if(event->modifiers() & Qt::AltModifier)	buttons |= NLMISC::altButton;
	
	return buttons;
}

uint32  CObjectViewerDialog::getNelButtons(QWheelEvent *event) {
	//nldebug("CObjectViewerDialog::getNelButtons");
	uint32 buttons = NLMISC::noButton;
	if(event->buttons() & Qt::LeftButton)	buttons |= NLMISC::leftButton;
	if(event->buttons() & Qt::RightButton)	buttons |= NLMISC::rightButton;
	if(event->buttons() & Qt::MidButton)	buttons |= NLMISC::middleButton;
	if(event->modifiers() & Qt::ControlModifier)	buttons |= NLMISC::ctrlButton;
	if(event->modifiers() & Qt::ShiftModifier)	buttons |= NLMISC::shiftButton;
	if(event->modifiers() & Qt::AltModifier)	buttons |= NLMISC::altButton;
	
	return buttons;
}
	
void CObjectViewerDialog::mousePressEvent(QMouseEvent *event) {
	//nldebug("CObjectViewerDialog::mousePressEvent");
	// Get relative positions.
	float fX = 1.0f - (float)event->pos().x() / this->width();
	float fY = 1.0f - (float)event->pos().y() / this->height();
	
	// Set the buttons currently pressed.
	NLMISC::TMouseButton buttons = (NLMISC::TMouseButton)getNelButtons(event);

	if(event->button() == Qt::LeftButton)
		Modules::objView().getDriver()->EventServer.postEvent(
			new NLMISC::CEventMouseDown( -fX, fY,
				(NLMISC::TMouseButton)(NLMISC::leftButton|(buttons&~(NLMISC::leftButton|NLMISC::middleButton|NLMISC::rightButton))), this));
	if(event->button() == Qt::MidButton)
		Modules::objView().getDriver()->EventServer.postEvent(
			new NLMISC::CEventMouseDown( -fX, fY,
				(NLMISC::TMouseButton)(NLMISC::middleButton|(buttons&~(NLMISC::middleButton|NLMISC::leftButton|NLMISC::rightButton))), this));
	if(event->button() == Qt::RightButton)
		Modules::objView().getDriver()->EventServer.postEvent(
			new NLMISC::CEventMouseDown( -fX, fY,
				(NLMISC::TMouseButton)(NLMISC::rightButton|(buttons&~(NLMISC::rightButton|NLMISC::leftButton|NLMISC::middleButton))), this));	
}

void CObjectViewerDialog::mouseReleaseEvent(QMouseEvent *event) {
	//nldebug("CObjectViewerDialog::mouseReleaseEvent");
	// Get relative positions.
	float fX = 1.0f - (float)event->pos().x() / this->width();
	float fY = 1.0f - (float)event->pos().y() / this->height();
	
	// Set the buttons currently pressed.
	NLMISC::TMouseButton buttons = (NLMISC::TMouseButton)getNelButtons(event);

	if(event->button() == Qt::LeftButton)
		Modules::objView().getDriver()->EventServer.postEvent(
			new NLMISC::CEventMouseUp( -fX, fY, NLMISC::leftButton, this));
	if(event->button() == Qt::MidButton)
		Modules::objView().getDriver()->EventServer.postEvent(
			new NLMISC::CEventMouseUp( -fX, fY, NLMISC::middleButton, this));
	if(event->button() == Qt::RightButton)
		Modules::objView().getDriver()->EventServer.postEvent(
			new NLMISC::CEventMouseUp( -fX, fY, NLMISC::rightButton, this));	
}

void CObjectViewerDialog::mouseMoveEvent(QMouseEvent *event) {
	//nldebug("CObjectViewerDialog::mouseMoveEvent");
	// Get relative positions.
	float fX = 1.0f - (float)event->pos().x() / this->width();
	float fY = 1.0f - (float)event->pos().y() / this->height();
	
	if ((fX == 0.5f) && (fY == 0.5f)) return;
	NLMISC::TMouseButton buttons = (NLMISC::TMouseButton)getNelButtons(event);
	Modules::objView().getDriver()->EventServer.postEvent(new NLMISC::CEventMouseMove(-fX, fY, buttons, this));
}

} /* namespace NLQT */
