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

// Project includes
#include "landscape_view.h"
#include "landscape_editor_constants.h"

#include "../core/icore.h"
#include "../core/core_constants.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QApplication>

namespace LandscapeEditor
{

LandscapeView::LandscapeView(QWidget *parent)
	: QGraphicsView(parent),
	  m_visibleGrid(true),
	  m_visibleText(false)
{
	//setDragMode(ScrollHandDrag);
	setTransformationAnchor(AnchorUnderMouse);
	setBackgroundBrush(QBrush(Qt::lightGray));
	//setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	//setRenderHints(QPainter::Antialiasing);
	//setCacheMode(QGraphicsView::CacheBackground);
	m_cellSize = 160;
	m_numSteps = 0;
	m_maxSteps = 20;

	//A modified version of centerOn(), handles special cases
	setCenter(QPointF(500.0, 500.0));
}

LandscapeView::~LandscapeView()
{
}

bool LandscapeView::isVisibleGrid() const
{
	return m_visibleGrid;
}

void LandscapeView::setVisibleGrid(bool visible)
{
	m_visibleGrid = visible;
	scene()->update();
}

void LandscapeView::setVisibleText(bool visible)
{
	m_visibleText = visible;
	scene()->update();
}

void LandscapeView::wheelEvent(QWheelEvent *event)
{
	/*	double numDegrees = event->delta() / 8.0;
		double numSteps = numDegrees / 15.0;
		double factor = std::pow(1.125, numSteps);
		if (factor > 1.0)
		{
			// check max scale view
			if (m_numSteps > m_maxSteps)
				return;
			++m_numSteps;
		}
		else
		{
			// check min scale view
			if (m_numSteps < -m_maxSteps)
				return;
			--m_numSteps;
		}
		scale(factor, factor);
		QGraphicsView::wheelEvent(event);*/

	//Get the position of the mouse before scaling, in scene coords
	QPointF pointBeforeScale(mapToScene(event->pos()));

	//Get the original screen centerpoint
	QPointF screenCenter = getCenter(); //CurrentCenterPoint; //(visRect.center());

	//Scale the view ie. do the zoom
	double scaleFactor = 1.15; //How fast we zoom
	if(event->delta() > 0)
	{
		//Zoom in
		scale(scaleFactor, scaleFactor);
	}
	else
	{
		//Zooming out
		scale(1.0 / scaleFactor, 1.0 / scaleFactor);
	}

	//Get the position after scaling, in scene coords
	QPointF pointAfterScale(mapToScene(event->pos()));

	//Get the offset of how the screen moved
	QPointF offset = pointBeforeScale - pointAfterScale;

	//Adjust to the new center for correct zooming
	QPointF newCenter = screenCenter + offset;
	setCenter(newCenter);
}

void LandscapeView::mousePressEvent(QMouseEvent *event)
{
	QGraphicsView::mousePressEvent(event);
	if (event->button() != Qt::MiddleButton)
		return;

	//For panning the view
	m_lastPanPoint = event->pos();
	setCursor(Qt::ClosedHandCursor);
}

void LandscapeView::mouseMoveEvent(QMouseEvent *event)
{
	if(!m_lastPanPoint.isNull())
	{
		//Get how much we panned
		QPointF delta = mapToScene(m_lastPanPoint) - mapToScene(event->pos());
		m_lastPanPoint = event->pos();

		//Update the center ie. do the pan
		setCenter(getCenter() + delta);
	}

	QGraphicsView::mouseMoveEvent(event);
}

void LandscapeView::mouseReleaseEvent(QMouseEvent *event)
{
	m_lastPanPoint = QPoint();
	QApplication::restoreOverrideCursor();
	QGraphicsView::mouseReleaseEvent(event);
}

void LandscapeView::resizeEvent(QResizeEvent *event)
{
	//Get the rectangle of the visible area in scene coords
	QRectF visibleArea = mapToScene(rect()).boundingRect();
	setCenter(visibleArea.center());

	//Call the subclass resize so the scrollbars are updated correctly
	QGraphicsView::resizeEvent(event);
}

void LandscapeView::setCenter(const QPointF &centerPoint)
{
	//Get the rectangle of the visible area in scene coords
	QRectF visibleArea = mapToScene(rect()).boundingRect();

	//Get the scene area
	QRectF sceneBounds = sceneRect();

	double boundX = visibleArea.width() / 2.0;
	double boundY = visibleArea.height() / 2.0;
	double boundWidth = sceneBounds.width() - 2.0 * boundX;
	double boundHeight = sceneBounds.height() - 2.0 * boundY;

	//The max boundary that the centerPoint can be to
	QRectF bounds(boundX, boundY, boundWidth, boundHeight);

	if(bounds.contains(centerPoint))
	{
		//We are within the bounds
		m_currentCenterPoint = centerPoint;
	}
	else
	{
		//We need to clamp or use the center of the screen
		if(visibleArea.contains(sceneBounds))
		{
			//Use the center of scene ie. we can see the whole scene
			m_currentCenterPoint = sceneBounds.center();
		}
		else
		{
			m_currentCenterPoint = centerPoint;

			//We need to clamp the center. The centerPoint is too large
			if (centerPoint.x() > bounds.x() + bounds.width())
			{
				m_currentCenterPoint.setX(bounds.x() + bounds.width());
			}
			else if(centerPoint.x() < bounds.x())
			{
				m_currentCenterPoint.setX(bounds.x());
			}

			if(centerPoint.y() > bounds.y() + bounds.height())
			{
				m_currentCenterPoint.setY(bounds.y() + bounds.height());
			}
			else if(centerPoint.y() < bounds.y())
			{
				m_currentCenterPoint.setY(bounds.y());
			}

		}
	}

	//Update the scrollbars
	centerOn(m_currentCenterPoint);
}

QPointF LandscapeView::getCenter() const
{
	return m_currentCenterPoint;
}

void LandscapeView::drawForeground(QPainter *painter, const QRectF &rect)
{
	QGraphicsView::drawForeground(painter, rect);

	if (!m_visibleGrid)
		return;

	painter->setPen(QPen(Qt::white, 0, Qt::SolidLine));
	drawGrid(painter, rect);

	if (!m_visibleText)
		return;

	if (m_numSteps > -m_maxSteps / 4)
	{
		painter->setPen(QPen(Qt::white, 0.5, Qt::SolidLine));
		//painter->setFont(QFont("Helvetica [Cronyx]", 12));
		drawZoneNames(painter, rect);
	}
}

void LandscapeView::drawGrid(QPainter *painter, const QRectF &rect)
{
	qreal left = m_cellSize * floor(rect.left() / m_cellSize);
	qreal top = m_cellSize * floor(rect.top() / m_cellSize);

	QVector<QLine> lines;

	// Calculate vertical lines
	while (left < rect.right())
	{
		lines.push_back(QLine(int(left), int(rect.bottom()), int(left), int(rect.top())));
		left += m_cellSize;
	}

	// Calculate horizontal lines
	while (top < rect.bottom())
	{
		lines.push_back(QLine(int(rect.left()), int(top), int(rect.right()), int(top)));
		top += m_cellSize;
	}

	// Draw lines
	painter->drawLines(lines);
}

void LandscapeView::drawZoneNames(QPainter *painter, const QRectF &rect)
{
	int leftSide = int(floor(rect.left() / m_cellSize));
	int rightSide = int(floor(rect.right() / m_cellSize));
	int topSide = int(floor(rect.top() / m_cellSize));
	int bottomSide = int(floor(rect.bottom() / m_cellSize));

	for (int i = leftSide; i < rightSide + 1; ++i)
	{
		for (int j = topSide; j < bottomSide + 1; ++j)
		{
			QString text = QString("%1_%2%3").arg(j).arg(QChar('A' + (i / 26))).arg(QChar('A' + (i % 26)));
			painter->drawText(i * m_cellSize + 5, j * m_cellSize + 15, text);
		}
	}
}

} /* namespace LandscapeEditor */
