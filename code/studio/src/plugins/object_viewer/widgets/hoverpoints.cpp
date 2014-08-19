// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2010  Dzmitry Kamiahin <dnk-88@tut.by>
// Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
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

#include "stdpch.h"
#include "hoverpoints.h"

#define printf

HoverPoints::HoverPoints(QWidget *widget, PointShape shape)
	: QObject(widget)
{
	m_widget = widget;
	widget->installEventFilter(this);
	widget->setAttribute(Qt::WA_AcceptTouchEvents);

	m_connectionType = CurveConnection;
	m_sortType = NoSort;
	m_shape = shape;
	m_pointPen = QPen(QColor(255, 255, 255, 191), 1);
	m_connectionPen = QPen(QColor(255, 255, 255, 127), 2);
	m_pointBrush = QBrush(QColor(191, 191, 191, 127));
	m_pointSize = QSize(11, 11);
	m_currentIndex = -1;
	m_editable = true;
	m_enabled = true;
	m_minCountPoints = 2;

	createGradient();

	connect(this, SIGNAL(pointsChanged(QPolygonF)),
			m_widget, SLOT(update()));
}


void HoverPoints::setEnabled(bool enabled)
{
	if (m_enabled != enabled)
	{
		m_enabled = enabled;
		m_widget->update();
	}
}


bool HoverPoints::eventFilter(QObject *object, QEvent *event)
{
	if (object == m_widget && m_enabled)
	{
		switch (event->type())
		{

		case QEvent::MouseButtonPress:
		{
			if (!m_fingerPointMapping.isEmpty())
				return true;
			QMouseEvent *me = (QMouseEvent *) event;

			QPointF clickPos = me->pos();
			int index = -1;
			for (int i=0; i<m_points.size(); ++i)
			{
				QPainterPath path;
				if (m_shape == CircleShape)
					path.addEllipse(pointBoundingRect(i));
				else
					path.addRect(pointBoundingRect(i));

				if (path.contains(clickPos))
				{
					index = i;
					break;
				}
			}

			if (me->button() == Qt::LeftButton)
			{
				if (index == -1)
				{
					if (!m_editable)
						return false;
					int pos = 0;
					// Insert sort for x or y
					if (m_sortType == XSort)
					{
						for (int i=0; i<m_points.size(); ++i)
							if (m_points.at(i).x() > clickPos.x())
							{
								pos = i;
								break;
							}
					}
					else if (m_sortType == YSort)
					{
						for (int i=0; i<m_points.size(); ++i)
							if (m_points.at(i).y() > clickPos.y())
							{
								pos = i;
								break;
							}
					}

					m_points.insert(pos, clickPos);
					m_locks.insert(pos, 0);
					m_currentIndex = pos;
					firePointChange();

				}
				else
				{
					m_currentIndex = index;
				}
				return true;

			}
			else if (me->button() == Qt::RightButton)
			{
				if (index >= 0 && m_editable)
				{
					if ((m_points.size() - 1) < m_minCountPoints)
						return true;
					if (m_locks[index] == 0)
					{
						m_locks.remove(index);
						m_points.remove(index);
					}
					firePointChange();
					return true;
				}
			}

		}
		break;

		case QEvent::MouseButtonRelease:
			if (!m_fingerPointMapping.isEmpty())
				return true;
			m_currentIndex = -1;
			break;

		case QEvent::MouseMove:
			if (!m_fingerPointMapping.isEmpty())
				return true;
			if (m_currentIndex >= 0)
				movePoint(m_currentIndex, ((QMouseEvent *)event)->pos());
			break;
		case QEvent::TouchBegin:
		case QEvent::TouchUpdate:
		{
			const QTouchEvent *const touchEvent = static_cast<const QTouchEvent *>(event);
			const QList<QTouchEvent::TouchPoint> points = touchEvent->touchPoints();
			const qreal pointSize = qMax(m_pointSize.width(), m_pointSize.height());
			Q_FOREACH (const QTouchEvent::TouchPoint &touchPoint, points)
			{
				const int id = touchPoint.id();
				switch (touchPoint.state())
				{
				case Qt::TouchPointPressed:
				{
					// find the point, move it
					QSet<int> activePoints = QSet<int>::fromList(m_fingerPointMapping.values());
					int activePoint = -1;
					qreal distance = -1;
					const int pointsCount = m_points.size();
					const int activePointCount = activePoints.size();
					if (pointsCount == 2 && activePointCount == 1)   // only two points
					{
						activePoint = activePoints.contains(0) ? 1 : 0;
					}
					else
					{
						for (int i=0; i<pointsCount; ++i)
						{
							if (activePoints.contains(i))
								continue;

							qreal d = QLineF(touchPoint.pos(), m_points.at(i)).length();
							if ((distance < 0 && d < 12 * pointSize) || d < distance)
							{
								distance = d;
								activePoint = i;
							}

						}
					}
					if (activePoint != -1)
					{
						m_fingerPointMapping.insert(touchPoint.id(), activePoint);
						movePoint(activePoint, touchPoint.pos());
					}
				}
				break;
				case Qt::TouchPointReleased:
				{
					// move the point and release
					QHash<int,int>::iterator it = m_fingerPointMapping.find(id);
					movePoint(it.value(), touchPoint.pos());
					m_fingerPointMapping.erase(it);
				}
				break;
				case Qt::TouchPointMoved:
				{
					// move the point
					const int pointIdx = m_fingerPointMapping.value(id, -1);
					if (pointIdx >= 0) // do we track this point?
						movePoint(pointIdx, touchPoint.pos());
				}
				break;
				default:
					break;
				}
			}
			if (m_fingerPointMapping.isEmpty())
			{
				event->ignore();
				return false;
			}
			else
			{
				return true;
			}
		}
		break;
		case QEvent::TouchEnd:
			if (m_fingerPointMapping.isEmpty())
			{
				event->ignore();
				return false;
			}
			return true;
			break;

		case QEvent::Resize:
		{
			QResizeEvent *e = (QResizeEvent *) event;
			if (e->oldSize().width() == 0 || e->oldSize().height() == 0)
				break;
			qreal stretch_x = e->size().width() / qreal(e->oldSize().width());
			qreal stretch_y = e->size().height() / qreal(e->oldSize().height());
			for (int i=0; i<m_points.size(); ++i)
			{
				QPointF p = m_points[i];
				movePoint(i, QPointF(p.x() * stretch_x, p.y() * stretch_y), false);
			}

			firePointChange();
			break;
		}

		case QEvent::Paint:
		{
			QWidget *that_widget = m_widget;
			m_widget = 0;
			QApplication::sendEvent(object, event);
			m_widget = that_widget;
			paintPoints();
			return true;
		}
		default:
			break;
		}
	}

	return false;
}

void HoverPoints::paintPoints()
{
	QPainter p;
	p.begin(m_widget);
	p.setRenderHint(QPainter::Antialiasing);

	p.setBrush(m_gradient);
	//p.setBrush(QColor(230,230,230));
	p.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	p.drawRoundedRect(QRect(1, 1, m_widget->width() - 2, m_widget->height() - 2), 4.0, 4.0);

	p.setBrush(QBrush());

	if (m_connectionPen.style() != Qt::NoPen && m_connectionType != NoConnection)
	{
		p.setPen(m_connectionPen);

		if (m_connectionType == CurveConnection)
		{
			QPainterPath path;
			path.moveTo(m_points.at(0));
			for (int i=1; i<m_points.size(); ++i)
			{
				QPointF p1 = m_points.at(i-1);
				QPointF p2 = m_points.at(i);
				qreal distance = p2.x() - p1.x();

				path.cubicTo(p1.x() + distance / 2, p1.y(),
							 p1.x() + distance / 2, p2.y(),
							 p2.x(), p2.y());
			}
			p.drawPath(path);
		}
		else
		{
			p.drawPolyline(m_points);
		}
	}

	p.setPen(m_pointPen);
	p.setBrush(m_pointBrush);

	for (int i=0; i<m_points.size(); ++i)
	{
		QRectF bounds = pointBoundingRect(i);
		if (m_shape == CircleShape)
			p.drawEllipse(bounds);
		else
			p.drawRect(bounds);
	}
}

static QPointF bound_point(const QPointF &point, const QRectF &bounds, int lock)
{
	QPointF p = point;

	qreal left = bounds.left();
	qreal right = bounds.right();
	qreal top = bounds.top();
	qreal bottom = bounds.bottom();

	if (p.x() < left || (lock & HoverPoints::LockToLeft)) p.setX(left);
	else if (p.x() > right || (lock & HoverPoints::LockToRight)) p.setX(right);

	/*    if (p.y() < top || (lock & HoverPoints::LockToTop)) p.setY(top);
	    else if (p.y() > bottom || (lock & HoverPoints::LockToBottom)) p.setY(bottom);
	*/

	return p;
}

void HoverPoints::setPoints(const QPolygonF &points)
{
	if (points.size() != m_points.size())
		m_fingerPointMapping.clear();
	m_points.clear();
	for (int i=0; i<points.size(); ++i)
		m_points << bound_point(points.at(i), boundingRect(), 0);

	m_locks.clear();
	if (m_points.size() > 0)
	{
		m_locks.resize(m_points.size());

		m_locks.fill(0);
	}
}


void HoverPoints::movePoint(int index, const QPointF &point, bool emitUpdate)
{
	m_points[index] = bound_point(point, boundingRect(), m_locks.at(index));
	if (emitUpdate)
		firePointChange();
}

void HoverPoints::createGradient()
{
	m_gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
	m_gradient.setCenter(0.45, 0.50);
	m_gradient.setFocalPoint(0.40, 0.45);
	m_gradient.setColorAt(0.0, QColor(175, 216, 252));
	m_gradient.setColorAt(0.4, QColor(151, 183, 220));
	m_gradient.setColorAt(0.8, QColor(86, 126, 191));
}

inline static bool x_less_than(const QPointF &p1, const QPointF &p2)
{
	return p1.x() < p2.x();
}


inline static bool y_less_than(const QPointF &p1, const QPointF &p2)
{
	return p1.y() < p2.y();
}

void HoverPoints::firePointChange()
{
	if (m_sortType != NoSort)
	{

		QPointF oldCurrent;
		if (m_currentIndex != -1)
		{
			oldCurrent = m_points[m_currentIndex];
		}

		if (m_sortType == XSort)
			qSort(m_points.begin(), m_points.end(), x_less_than);
		else if (m_sortType == YSort)
			qSort(m_points.begin(), m_points.end(), y_less_than);

		// Compensate for changed order...
		if (m_currentIndex != -1)
		{
			for (int i=0; i<m_points.size(); ++i)
			{
				if (m_points[i] == oldCurrent)
				{
					m_currentIndex = i;
					break;
				}
			}
		}
	}
	Q_EMIT pointsChanged(m_points);
}
