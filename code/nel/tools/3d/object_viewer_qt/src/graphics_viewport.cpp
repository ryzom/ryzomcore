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

CGraphicsViewport::CGraphicsViewport(QWidget *parent)
	: QNLWidget(parent)
{
	this->setStatusTip("Status ready");
}

CGraphicsViewport::~CGraphicsViewport()
{

}

void CGraphicsViewport::init()
{
	//H_AUTO2
	nldebug("CGraphicsViewport::init");

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
	makeCurrent();
#endif // defined(NL_OS_UNIX) && !defined(NL_OS_MAC)

	Modules::objView().init((nlWindow)winId(), width(), height());
	Modules::psEdit().init();

	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
}

void CGraphicsViewport::release()
{
	//H_AUTO2
	nldebug("CGraphicsViewport::release");

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

void CGraphicsViewport::resizeEvent(QResizeEvent *resizeEvent)
{
	QWidget::resizeEvent(resizeEvent);
	if (Modules::objView().getDriver())
		Modules::objView().setSizeViewport(resizeEvent->size().width(), resizeEvent->size().height());
}

#if defined(NL_OS_MAC)
// Qt does not provide wheel events through winEvent() and macEvent() (but it 
// does through x11Event(), which is inconsistent...)  
// Workaround is to handle wheel events like implemented below.
//
// TODO: this is not a clean solution, because all but wheel events are 
// handled using winEvent(), x11Event(), macEvent(). But this seems to be a 
// limitation of current (4.7.1) Qt versions. (see e.g. qapplication_mac.mm)
void CGraphicsViewport::wheelEvent(QWheelEvent *event)
{
	// Get relative positions.
	float fX = 1.0f - (float)event->pos().x() / this->width();
	float fY = 1.0f - (float)event->pos().y() / this->height();

	// Get the buttons currently pressed.
	uint32 buttons = NLMISC::noButton;
	if(event->buttons() & Qt::LeftButton)        buttons |= NLMISC::leftButton;
	if(event->buttons() & Qt::RightButton)       buttons |= NLMISC::rightButton;
	if(event->buttons() & Qt::MidButton)         buttons |= NLMISC::middleButton;
	if(event->modifiers() & Qt::ControlModifier) buttons |= NLMISC::ctrlButton;
	if(event->modifiers() & Qt::ShiftModifier)   buttons |= NLMISC::shiftButton;
	if(event->modifiers() & Qt::AltModifier)     buttons |= NLMISC::altButton;

	if(event->delta() > 0)
		Modules::objView().getDriver()->EventServer.postEvent(
			new NLMISC::CEventMouseWheel(-fX, fY, (NLMISC::TMouseButton)buttons, true, NULL));
	else
		Modules::objView().getDriver()->EventServer.postEvent(
			new NLMISC::CEventMouseWheel(-fX, fY, (NLMISC::TMouseButton)buttons, false, NULL));
}
#endif // defined(NL_OS_MAC)


#if defined(NL_OS_WINDOWS)

typedef bool (*winProc)(NL3D::IDriver *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

bool CGraphicsViewport::winEvent(MSG *message, long *result)
{
	if (Modules::objView().getDriver() && Modules::objView().getDriver()->isActive())
	{
		NL3D::IDriver *driver = dynamic_cast<NL3D::CDriverUser *>(Modules::objView().getDriver())->getDriver();
		if (driver)
		{
			winProc proc = (winProc)driver->getWindowProc();
			return proc(driver, message->hwnd, message->message, message->wParam, message->lParam);
		}
	}

	return false;
}

#elif defined(NL_OS_MAC)

typedef bool (*cocoaProc)(NL3D::IDriver *, const void *e);

bool CGraphicsViewport::macEvent(EventHandlerCallRef caller, EventRef event)
{
	if(caller)
		nlerror("You are using QtCarbon! Only QtCocoa supported, please upgrade Qt");

	if (Modules::objView().getDriver() && Modules::objView().getDriver()->isActive())
	{
		NL3D::IDriver *driver = dynamic_cast<NL3D::CDriverUser *>(Modules::objView().getDriver())->getDriver();
		if (driver)
		{
			cocoaProc proc = (cocoaProc)driver->getWindowProc();
			return proc(driver, event);
		}
	}

	return false;
}

#elif defined(NL_OS_UNIX)

typedef bool (*x11Proc)(NL3D::IDriver *drv, XEvent *e);

bool CGraphicsViewport::x11Event(XEvent *event)
{
	if (Modules::objView().getDriver() && Modules::objView().getDriver()->isActive())
	{
		NL3D::IDriver *driver = dynamic_cast<NL3D::CDriverUser *>(Modules::objView().getDriver())->getDriver();
		if (driver)
		{
			x11Proc proc = (x11Proc)driver->getWindowProc();
			return proc(driver, event);
		}
	}

	return false;
}
#endif

} /* namespace NLQT */

