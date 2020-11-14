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

	CObjectViewerDialog::CObjectViewerDialog(QWidget *parent) :QDockWidget(parent),
		_nlw(0)
	{
		_ui.setupUi(this);

		if (Modules::objViewInt()) 
		{
			_nlw = dynamic_cast<QNLWidget*>(Modules::objViewInt()->getWidget());
			//_nlw->setObjectName(QString::fromUtf8("nlwidget"));
			_ui.gridLayout->addWidget(_nlw, 0, 0);
		}
	}

	CObjectViewerDialog::~CObjectViewerDialog()
	{
	}

	void CObjectViewerDialog::setVisible(bool visible)
	{
		// called by show()
		// code assuming visible window needed to init the 3d driver
		nldebug("%d", visible);
		if (visible)
		{
			QDockWidget::setVisible(true);
			//_mainTimer->start(25);
			//updateInitialization(true);
			//_nlw->show();
		}
		else
		{
			//updateInitialization(false);
			QDockWidget::setVisible(false);
		}
	}

	void CObjectViewerDialog::topLevelChanged ( bool topLevel )
	{
		//nldebug("topLevel:%d",topLevel);
		//_georges->init();
	}

	QAction *CObjectViewerDialog::createSaveScreenshotAction(QObject *parent)
	{
		QAction *action = new QAction(parent);
		connect(action, SIGNAL(triggered()), _nlw, SLOT(saveScreenshot()));
		return action;
	}

	QAction *CObjectViewerDialog::createSetBackgroundColor(QObject *parent)
	{
		QAction *action = new QAction(parent);
		connect(action, SIGNAL(triggered()), _nlw, SLOT(setBackgroundColor()));
		return action;
	}

	void CObjectViewerDialog::resizeEvent(QResizeEvent *resizeEvent)
	{
		//nldebug("%d %d",_nlw->width(), _nlw->height());
		QDockWidget::resizeEvent(resizeEvent);
		if (Modules::objViewInt())
		{
			if (Modules::objViewInt()->getDriver())
				Modules::objViewInt()->setSizeViewport(resizeEvent->size().width(), resizeEvent->size().height());
		}
		// The OpenGL driver does not resize automatically.
		// The Direct3D driver breaks the window mode to include window borders when calling setMode windowed.

		// Resizing the window after switching drivers a few times becomes slow.
		// There is probably something inside the drivers not being released properly.
	}

} /* namespace NLQT */
