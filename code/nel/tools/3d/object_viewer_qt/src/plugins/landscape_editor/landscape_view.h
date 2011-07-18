// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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

#ifndef LANDSCAPE_VIEW_H
#define LANDSCAPE_VIEW_H

// Project includes
#include "landscape_editor_global.h"

// Qt includes
#include <QtGui/QGraphicsView>
#include <QtGui/QWheelEvent>

namespace LandscapeEditor
{

class LANDSCAPE_EDITOR_EXPORT LandscapeView: public QGraphicsView
{
	Q_OBJECT

public:
	LandscapeView(QWidget *parent = 0);
	virtual ~LandscapeView();

	bool isVisibleGrid() const;

public Q_SLOTS:
	void setVisibleGrid(bool visible);

private Q_SLOTS:
protected:
	virtual void wheelEvent(QWheelEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void drawForeground(QPainter *painter, const QRectF &rect);

	void drawGrid(QPainter *painter, const QRectF &rect);
	void drawZoneNames(QPainter *painter, const QRectF &rect);
private:

	bool m_visibleGrid;
	int m_numSteps, m_maxSteps;
	int m_cellSize;
	bool m_moveMouse;
}; /* class LandscapeView */

} /* namespace LandscapeEditor */

#endif // LANDSCAPE_VIEW_H
