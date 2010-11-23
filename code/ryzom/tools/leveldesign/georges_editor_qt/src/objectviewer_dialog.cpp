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
#include <nel/3d/driver_user.h>

// Project includes
#include "modules.h"

using namespace std;
using namespace NL3D;

namespace NLQT 
{

	CObjectViewerDialog::CObjectViewerDialog(QWidget *parent)
		: _isGraphicsInitialized(false), _isGraphicsEnabled(false), QDockWidget(parent)
	{
		_ui.setupUi(this);

//		_nlw = new QNLWidget(_ui.dockWidgetContents);
		_nlw = &Modules::objViewWid();
		//_nlw->setObjectName(QString::fromUtf8("nlwidget"));
		_ui.gridLayout->addWidget(_nlw, 0, 0);

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
		_mainTimer->start(25); // 25fps
	}

	CObjectViewerDialog::~CObjectViewerDialog() 
	{
		_mainTimer->stop();
	}

	void CObjectViewerDialog::init()
	{
		connect(this, SIGNAL(topLevelChanged(bool)),
			this, SLOT(topLevelChanged(bool)));
		//H_AUTO2
		//nldebug("%d %d %d",_nlw->winId(), width(), height());

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
		dynamic_cast<QNLWidget*>(widget())->makeCurrent();
#endif // defined(NL_OS_UNIX) && !defined(NL_OS_MAC)

		Modules::objView().init((nlWindow)_nlw->winId(), width(), height());
		setMouseTracking(true);
	}

	void CObjectViewerDialog::release()
	{
		nldebug("");

		Modules::objView().release();
	}

	void CObjectViewerDialog::setVisible(bool visible)
	{
		// called by show()
		// code assuming visible window needed to init the 3d driver
		nldebug("%d", visible);
		if (visible)
		{
			QDockWidget::setVisible(true);
			_mainTimer->start(25);
			//updateInitialization(true);
			//_nlw->show();
		}
		else
		{
			//updateInitialization(false);
			QDockWidget::setVisible(false);
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
					//_isGraphicsInitialized = false;
					//release();
					_mainTimer->stop();
					//done = false;
				}
			}
			else
			{
				if (wantGraphics)
				{
					init();
					_isGraphicsInitialized = true;
					_mainTimer->start(25);
					//done = false;
				}
			}
		}
		while (!done);
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
		//nldebug("%d %d",winId(), _nlw->winId());
		// winId changing when re/docking
		//Modules::georges().init();
		//Modules::objView().reinit((nlWindow)_nlw->winId(), _nlw->width(), _nlw->height());
	}

	void CObjectViewerDialog::resizeEvent(QResizeEvent *resizeEvent)
	{
		//nldebug("%d %d",_nlw->width(), _nlw->height());
		QDockWidget::resizeEvent(resizeEvent);
		if (Modules::objView().getDriver())
			Modules::objView().setSizeViewport(resizeEvent->size().width(), resizeEvent->size().height());
		// The OpenGL driver does not resize automatically.
		// The Direct3D driver breaks the window mode to include window borders when calling setMode windowed.

		// Resizing the window after switching drivers a few times becomes slow.
		// There is probably something inside the drivers not being released properly.
	}

} /* namespace NLQT */
