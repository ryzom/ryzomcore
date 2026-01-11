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

#include <nel/misc/types_nl.h>
#include "graphics_viewport.h"

// STL includes

// Qt includes
#include <QtGui/QAction>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/hierarchical_timer.h>
#include <nel/misc/file.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/path.h>
#include <nel/misc/i18n.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>

// Project includes
#include "internationalization.h"
#include "graphics_config.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

namespace NLQT {

namespace {

QString nli18n(const char *label)
{
	return QString::fromUtf16(CI18N::get(label).c_str());
}

} /* anonymous namespace */

CGraphicsViewport::CGraphicsViewport(QWidget *parent) 
	: QWidget(parent),
	m_GraphicsConfig(NULL), 
	m_Driver(NULL), 
	m_TextContext(NULL)
{
	
}

CGraphicsViewport::~CGraphicsViewport()
{
	
}

void CGraphicsViewport::init(CGraphicsConfig *graphicsConfig)
{
	//H_AUTO2
	nldebug("CGraphicsViewport::init");

	// copy parameters
	m_GraphicsConfig = graphicsConfig;
	
	// check stuff we need
	nlassert(m_GraphicsConfig);

	// create the driver
	nlassert(!m_Driver);
	m_Direct3D = false;
	std::string driver = m_GraphicsConfig->getGraphicsDriver();
	if (driver == "Direct3D") m_Direct3D = true; //m_Driver = Direct3D;
	else if (driver == "OpenGL") m_Direct3D = false; //m_Driver = OpenGL;
	else
	{
		nlwarning("Invalid driver specified, defaulting to OpenGL");
		//m_Configuration->getConfigFile().getVar("GraphicsDriver").forceAsString("OpenGL");
		//m_Driver = OpenGL;
	}
	m_Driver = UDriver::createDriver(NULL, m_Direct3D, NULL);
	nlassert(m_Driver);

	// initialize the window with config file values
	m_Driver->setDisplay(winId(), NL3D::UDriver::CMode(width(), height(), 32));

	// register config callbacks
	connect(m_GraphicsConfig, SIGNAL(onBackgroundColor(NLMISC::CRGBA)), 
		this, SLOT(cfcbBackgroundColor(NLMISC::CRGBA)));
	m_BackgroundColor = m_GraphicsConfig->getBackgroundColor();

	// set the cache size for the font manager(in bytes)
	m_Driver->setFontManagerMaxMemory(2097152);
	
	// create the text context
	nlassert(!m_TextContext);
	m_TextContext = m_Driver->createTextContext(CPath::lookup(
		m_GraphicsConfig->getFontName()));
	nlassert(m_TextContext);
	connect(m_GraphicsConfig, SIGNAL(onFontShadow(bool)), 
		this, SLOT(cfcbFontShadow(bool)));
	m_TextContext->setShaded(m_GraphicsConfig->getFontShadow());	
}

void CGraphicsViewport::release()
{
	//H_AUTO2
	nldebug("CGraphicsViewport::release");

	// release text context
	nlassert(m_TextContext);
	disconnect(m_GraphicsConfig, SIGNAL(onFontShadow(bool)), 
		this, SLOT(cfcbFontShadow(bool)));
	m_Driver->deleteTextContext(m_TextContext);
	m_TextContext = NULL;

	// release driver
	nlassert(m_Driver);
	disconnect(m_GraphicsConfig, SIGNAL(onBackgroundColor(NLMISC::CRGBA)), 
		this, SLOT(cfcbBackgroundColor(NLMISC::CRGBA)));
	m_Driver->release();
	delete m_Driver;
	m_Driver = NULL;

	// reset parameters
	m_GraphicsConfig = NULL;
}

void CGraphicsViewport::updateInput()
{
	m_Driver->EventServer.pump();
}

void CGraphicsViewport::renderDriver()
{
	m_Driver->clearBuffers(m_BackgroundColor);
}

void CGraphicsViewport::renderDebug2D()
{
	m_TextContext->setColor(NL3D::CRGBA (255, 255, 255));
	m_TextContext->setFontSize(40);
	m_TextContext->setHotSpot(NL3D::UTextContext::BottomLeft);
	m_TextContext->printAt(0.3f, 0.5f, std::string("NeL Qt"));
}

void CGraphicsViewport::cfcbBackgroundColor(NLMISC::CRGBA backgroundColor)
{
	m_BackgroundColor = backgroundColor;
}

void CGraphicsViewport::cfcbFontShadow(bool fontShadow)
{
	m_TextContext->setShaded(fontShadow);
}

QAction *CGraphicsViewport::createSaveScreenshotAction(QObject *parent)
{
	QAction *action = new QAction(parent);
	connect(action, SIGNAL(triggered()), this, SLOT(saveScreenshot()));
	return action;
}

void CGraphicsViewport::saveScreenshot()
{
	saveScreenshot(
		m_GraphicsConfig->getScreenshotName(),
		m_GraphicsConfig->getScreenshotJPG(),
		m_GraphicsConfig->getScreenshotPNG(),
		m_GraphicsConfig->getScreenshotTGA());
}

void CGraphicsViewport::saveScreenshot(const string &name, bool jpg, bool png, bool tga)
{
	//H_AUTO2

	// FIXME: create screenshot path if it doesn't exist!
	
	// empty bitmap
	CBitmap bitmap;
	// copy the driver buffer to the bitmap
	m_Driver->getBuffer(bitmap);
	// create the file name
	string filename = std::string("./")
		+ m_GraphicsConfig->getScreenshotPath()
		+ std::string("/") + name;
	// write the bitmap as a jpg, png or tga to the file
	if (jpg)
	{
		string newfilename = CFile::findNewFile(filename + ".jpg");
		COFile outputFile(newfilename);
		bitmap.writeJPG(outputFile, 100);
		nlinfo("Screenshot '%s' saved", newfilename.c_str());
	}
	if (png)
	{
		string newfilename = CFile::findNewFile(filename + ".png");
		COFile outputFile(newfilename);
		bitmap.writePNG(outputFile, 24);
		nlinfo("Screenshot '%s' saved", newfilename.c_str());
	}
	if (tga)
	{
		string newfilename = CFile::findNewFile(filename + ".tga");
		COFile outputFile(newfilename);
		bitmap.writeTGA(outputFile, 24, false);
		nlinfo("Screenshot '%s' saved", newfilename.c_str());
	}
}

void CGraphicsViewport::resizeEvent(QResizeEvent *resizeEvent)
{
	QWidget::resizeEvent(resizeEvent);
	if (m_Driver && !m_Direct3D)
	{
		m_Driver->setMode(UDriver::CMode(resizeEvent->size().width(), resizeEvent->size().height(), 32));
	}

	// The OpenGL driver does not resize automatically.
	// The Direct3D driver breaks the window mode to include window borders when calling setMode windowed.

	// Resizing the window after switching drivers a few times becomes slow.
	// There is probably something inside the drivers not being released properly.
}

} /* namespace NLQT */

/* end of file */
