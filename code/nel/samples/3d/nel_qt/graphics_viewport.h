/**
 * CGraphicsViewport
 * $Id: graphics_viewport.h 2247 2010-02-15 21:16:38Z kaetemi $
 * \file graphics_viewport.h
 * \brief CGraphicsViewport
 * \date 2010-02-06 10:11GMT
 * \author Jan Boon (Kaetemi)
 */

/* 
 * Copyright (C) 2010  by authors
 * 
 * This file is part of NEL QT.
 * NEL QT is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * NEL QT is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with NEL QT; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef NLQT_GRAPHICS_VIEWPORT_H
#define NLQT_GRAPHICS_VIEWPORT_H
#include <nel/misc/types_nl.h>

// STL includes

// Qt includes
#include <QtGui/QWidget>

// NeL includes
#include <nel/misc/config_file.h>
#include <nel/misc/rgba.h>

// Project includes

class QAction;

namespace NL3D {
	class UDriver;
	class UTextContext;
	class UScene;
}

namespace NLQT {
	class CGraphicsConfig;
	class CInternationalization;

/**
 * CGraphicsViewport
 * \brief CGraphicsViewport
 * \date 2010-02-06 10:11GMT
 * \author Jan Boon (Kaetemi)
 */
class CGraphicsViewport : public QWidget
{
	Q_OBJECT

public:
	CGraphicsViewport(QWidget *parent);
	virtual ~CGraphicsViewport();

	virtual QPaintEngine* paintEngine() const { return NULL; }

	void init(CGraphicsConfig *graphicsConfig);
	void release();

	void updateInput();
	void renderDriver();
	void renderDebug2D();

	QAction *createSaveScreenshotAction(QObject *parent);

	void saveScreenshot(const std::string &name, bool jpg, bool png, bool tga);

	inline NL3D::UDriver *getDriver() { return m_Driver; }
	inline NL3D::UTextContext *getTextContext() { return m_TextContext; }
	inline NL3D::UScene *getScene() { return m_Scene; }

public slots:		
	void saveScreenshot();

private slots:
	void cfcbBackgroundColor(NLMISC::CRGBA backgroundColor);
	void cfcbFontShadow(bool fontShadow);

private:
	virtual void resizeEvent(QResizeEvent *resizeEvent);

private:
	NLMISC::CRGBA m_BackgroundColor;

	CGraphicsConfig *m_GraphicsConfig;

	NL3D::UDriver *m_Driver;
	NL3D::UTextContext *m_TextContext;
	NL3D::UScene *m_Scene;

	bool m_Direct3D;

private:
	CGraphicsViewport(const CGraphicsViewport &);
	CGraphicsViewport &operator=(const CGraphicsViewport &);
	
}; /* class CGraphicsViewport */

} /* namespace NLQT */

#endif /* #ifndef NLQT_GRAPHICS_VIEWPORT_H */

/* end of file */
