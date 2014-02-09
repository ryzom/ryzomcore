// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

/**
@class LandscapeView
@brief Provides graphics view for viewing zone regions.
@details Also provides zooming, panning and displaying grid
*/
class LANDSCAPE_EDITOR_EXPORT LandscapeView: public QGraphicsView
{
	Q_OBJECT

public:
	explicit LandscapeView(QWidget *parent = 0);
	virtual ~LandscapeView();

	//Set the current centerpoint in the
	void setCenter(const QPointF &centerPoint);
	QPointF getCenter() const;

	bool isVisibleGrid() const;

public Q_SLOTS:

	/// Enable/disable displaying grid.
	void setVisibleGrid(bool visible);

	/// Enable/disable displaying text(coord.) above each zone bricks.
	void setVisibleText(bool visible);

private Q_SLOTS:
protected:
	//Take over the interaction
	virtual void wheelEvent(QWheelEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void drawForeground(QPainter *painter, const QRectF &rect);
	virtual void resizeEvent(QResizeEvent *event);

	void drawGrid(QPainter *painter, const QRectF &rect);
	void drawZoneNames(QPainter *painter, const QRectF &rect);
private:

	bool m_visibleGrid, m_visibleText;
	qreal m_maxView, m_minView, m_maxViewText;
	int m_cellSize;

	//Holds the current centerpoint for the view, used for panning and zooming
	QPointF m_currentCenterPoint;

	//From panning the view
	QPoint m_lastPanPoint;
}; /* class LandscapeView */

} /* namespace LandscapeEditor */

#endif // LANDSCAPE_VIEW_H
