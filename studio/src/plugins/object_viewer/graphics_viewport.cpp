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

#include "stdpch.h"
#include "graphics_viewport.h"
#include "../core/Nel3DWidget/nel3d_widget.h"

// STL includes

// Qt includes
#include <QtGui/QAction>
#include <QtGui/QResizeEvent>
#include <QtGui/QColorDialog>
#include <QtGui/QFileDialog>

// NeL includes
#include <nel/misc/rgba.h>

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

CGraphicsViewport::CGraphicsViewport(QObject *parent)
	: QObject(parent)
{
	w = new Nel3DWidget();
	connect( w, SIGNAL( resize( int, int ) ), this, SLOT( onResize( int, int ) ) );
}

CGraphicsViewport::~CGraphicsViewport()
{
	disconnect( w, SIGNAL( resize( int, int ) ), this, SLOT( onResize( int, int ) ) );
	delete w;
	w = NULL;
}

void CGraphicsViewport::init()
{
	//H_AUTO2
	nldebug("CGraphicsViewport::init");

	w->init();
	Modules::objView().init( w->getDriver() );
	Modules::psEdit().init();
	Modules::veget().init();

	w->setMouseTracking(true);
	w->setFocusPolicy(Qt::StrongFocus);
}

void CGraphicsViewport::release()
{
	//H_AUTO2
	nldebug("CGraphicsViewport::release");

	Modules::veget().release();
	Modules::psEdit().release();
	Modules::objView().release();
}


QAction *CGraphicsViewport::createSaveScreenshotAction(QObject *parent)
{
	QAction *action = new QAction(parent);
	connect(action, SIGNAL(triggered()), this, SLOT(saveScreenshot()));
	return action;
}

QAction *CGraphicsViewport::createSetBackgroundColor(QObject *parent)
{
	QAction *action = new QAction(parent);
	connect(action, SIGNAL(triggered()), this, SLOT(setBackgroundColor()));
	return action;
}

QWidget* CGraphicsViewport::widget()
{
	return w;
}

void CGraphicsViewport::saveScreenshot()
{
	Modules::objView().saveScreenshot("screenshot", false, true, false);
}

void CGraphicsViewport::setBackgroundColor()
{
	QColor color = QColorDialog::getColor(QColor(Modules::objView().getBackgroundColor().R,
										  Modules::objView().getBackgroundColor().G,
										  Modules::objView().getBackgroundColor().B));
	if (color.isValid())
		Modules::objView().setBackgroundColor(NLMISC::CRGBA(color.red(), color.green(), color.blue()));
}

void CGraphicsViewport::onResize( int width, int height )
{
	if (Modules::objView().getDriver())
		Modules::objView().setSizeViewport( width, height );
}

} /* namespace NLQT */

