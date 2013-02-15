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

#ifndef HOVERPOINTS_H
#define HOVERPOINTS_H

#include <QtGui/QtGui>

QT_FORWARD_DECLARE_CLASS(QBypassWidget)

class HoverPoints : public QObject
{
	Q_OBJECT
public:
	enum PointShape
	{
		CircleShape,
		RectangleShape
	};

	enum LockType
	{
		LockToLeft   = 0x01,
		LockToRight  = 0x02,
		LockToTop    = 0x04,
		LockToBottom = 0x08
	};

	enum SortType
	{
		NoSort,
		XSort,
		YSort
	};

	enum ConnectionType
	{
		NoConnection,
		LineConnection,
		CurveConnection
	};

	HoverPoints(QWidget *widget, PointShape shape);

	bool eventFilter(QObject *object, QEvent *event);

	void paintPoints();

	inline QRectF boundingRect() const;
	void setBoundingRect(const QRectF &boundingRect)
	{
		m_bounds = boundingRect;
	}

	QPolygonF points() const
	{
		return m_points;
	}
	void setPoints(const QPolygonF &points);

	QSizeF pointSize() const
	{
		return m_pointSize;
	}
	void setPointSize(const QSizeF &size)
	{
		m_pointSize = size;
	}

	SortType sortType() const
	{
		return m_sortType;
	}
	void setSortType(SortType sortType)
	{
		m_sortType = sortType;
	}

	ConnectionType connectionType() const
	{
		return m_connectionType;
	}
	void setConnectionType(ConnectionType connectionType)
	{
		m_connectionType = connectionType;
	}

	void setConnectionPen(const QPen &pen)
	{
		m_connectionPen = pen;
	}
	void setShapePen(const QPen &pen)
	{
		m_pointPen = pen;
	}
	void setShapeBrush(const QBrush &brush)
	{
		m_pointBrush = brush;
	}

	void setPointLock(int pos, LockType lock)
	{
		m_locks[pos] = lock;
	}

	void setEditable(bool editable)
	{
		m_editable = editable;
	}
	bool editable() const
	{
		return m_editable;
	}

public Q_SLOTS:
	void setEnabled(bool enabled);
	void setDisabled(bool disabled)
	{
		setEnabled(!disabled);
	}

Q_SIGNALS:
	void pointsChanged(const QPolygonF &points);

public:
	void firePointChange();

private:
	inline QRectF pointBoundingRect(int i) const;
	void movePoint(int i, const QPointF &newPos, bool emitChange = true);
	void createGradient();

	QWidget *m_widget;

	QPolygonF m_points;
	QRectF m_bounds;
	PointShape m_shape;
	SortType m_sortType;
	ConnectionType m_connectionType;

	QVector<uint> m_locks;

	QSizeF m_pointSize;
	int m_currentIndex;
	int m_minCountPoints;
	bool m_editable;
	bool m_enabled;

	QHash<int, int> m_fingerPointMapping;

	QPen m_pointPen;
	QBrush m_pointBrush;
	QPen m_connectionPen;
	QRadialGradient m_gradient;
};


inline QRectF HoverPoints::pointBoundingRect(int i) const
{
	QPointF p = m_points.at(i);
	qreal w = m_pointSize.width();
	qreal h = m_pointSize.height();
	qreal x = p.x() - w / 2;
	qreal y = p.y() - h / 2;
	return QRectF(x, y, w, h);
}

inline QRectF HoverPoints::boundingRect() const
{
	if (m_bounds.isEmpty())
		return m_widget->rect();
	else
		return m_bounds;
}

#endif // HOVERPOINTS_H
