// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "qnelwidget.h"

#if defined(Q_WS_WIN)
#include <windows.h>	// needed for WindowFromDC()
#else
#include <Qt/qx11info_x11.h>
#include <X11/Xlib.h>
#endif

#include <nel/misc/types_nl.h>
#include <nel/misc/debug.h>
#include <nel/3d/nelu.h>
#include <nel/3d/init_3d.h>
#include <nel/3d/register_3d.h>
#include <nel/misc/path.h>

void QNelWidget::init()
{
	nlinfo("start init.");
	// do something, like initialize the basic system.
	NL3D::init3d();
	NL3D::CScene::registerBasics();
	NL3D::registerSerial3d();
	nlinfo("end init.");
}

void QNelWidget::initializeGL()
{
	// initialize NeL context if needed
	if (!NLMISC::INelContext::isContextInitialised())
		new NLMISC::CApplicationContext;

	nlinfo("start initialize gl");
	NLMISC::CPath::addSearchPath(".");

	// The viewport
	NL3D::CViewport viewport;

	// Create a dummy driver.
	//NL3D::IDriver *driver=NL3D::CDRU::createGlDriver();

	//void *windowId = (void*)(this->parentWidget()->winId());
	// Init NELU
	if (!NL3D::CNELU::init(width(), height(), viewport, 32, true, (void *)winId(), false, false))
	{
		return;
	}

	//_SceneRoot= (NL3D::CTransform*)NL3D::CNELU::Scene->createModel(NL3D::TransformId);
	NL3D::CTransformShape *sphere = NL3D::CNELU::Scene->createInstance("sphere01.shape");

	
	fontManager.setMaxMemory(2000000);
	
	tc.init(NL3D::CNELU::Driver, &fontManager);
	tc.setFontGenerator (NLMISC::CPath::lookup("beteckna.ttf"));
	nlinfo("end initialize gl");
}

void QNelWidget::paintGL()
{
	nlinfo("start painting gl");
	NL3D::CNELU::clearBuffers(NL3D::CRGBA(0,0,0));
	tc.setColor(NL3D::CRGBA (0, 0, 255));
	tc.setFontSize(40);
	tc.setHotSpot(NL3D::CComputedString::BottomLeft);
	tc.printAt(0.3f, 0.5f, std::string("NeL"));
	nlinfo("end painting gl");
}

void QNelWidget::resizeGL(int width, int height)
{
	nlinfo("start resize gl");
	// handle moves or resizes.
	nlinfo("end resize gl");
}

void QNelWidget::swapBuffers() {
	nlinfo("starting rendering.");
	// render a frame
	NL3D::CNELU::swapBuffers();
	nlinfo("finished rendering.");
}