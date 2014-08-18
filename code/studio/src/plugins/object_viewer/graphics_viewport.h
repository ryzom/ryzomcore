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

#ifndef GRAPHICS_VIEWPORT_H
#define GRAPHICS_VIEWPORT_H

#include <nel/misc/types_nl.h>
#include <nel/misc/event_emitter.h>

// STL includes

// Qt includes
#include <QObject>

// NeL includes

// Project includes
class QWidget;
class QAction;
class Nel3DWidget;

namespace NLQT
{

/**
@class CGraphicsViewport
@brief Responsible for interaction between Qt and NeL. Initializes CObjectViewer, CParticleEditor and CVegetableEditor subsystem.
*/
class CGraphicsViewport : public QObject, public NLMISC::IEventEmitter
{
	Q_OBJECT

public:
	CGraphicsViewport(QObject *parent);
	virtual ~CGraphicsViewport();

	void init();
	void release();

	QAction *createSaveScreenshotAction(QObject *parent);
	QAction *createSetBackgroundColor(QObject *parent);

	QWidget* widget();

private Q_SLOTS:
	void saveScreenshot();
	void setBackgroundColor();

	void submitEvents(NLMISC::CEventServer &server, bool allWindows) { }
	void emulateMouseRawMode(bool) { }

	void onResize( int width, int height );

private:
	CGraphicsViewport(const CGraphicsViewport &);
	CGraphicsViewport &operator=(const CGraphicsViewport &);


	Nel3DWidget *w;

}; /* class CGraphicsViewport */

} /* namespace NLQT */


#endif // GRAPHICS_VIEWPORT_H
