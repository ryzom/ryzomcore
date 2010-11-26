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
#include "graphics_info_widget.h"

// Qt includes
#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QMouseEvent>

// STL includes

namespace NLQT
{

const int directionSize = 35;

CGraphicsInfoWidget::CGraphicsInfoWidget(QWidget *parent)
	: QWidget(parent)
{
	_color = Qt::white;
	_mode = Mode::Color;
	_x = 0.0;
	_y = 0.0;
	_text = "";
	_braceMode = false;
}

CGraphicsInfoWidget::~CGraphicsInfoWidget()
{
}

void CGraphicsInfoWidget::setMode(int mode)
{
	_mode = mode;
}

void CGraphicsInfoWidget::setColor(const QColor &color)
{
	_color = color;
	repaint();
}

void CGraphicsInfoWidget::setVector(float x, float y)
{
	_mode = Mode::Direction;
	_x = x;
	_y = y;
	repaint();
}

void CGraphicsInfoWidget::setText(const QString &text)
{
	_text = text;
}

void CGraphicsInfoWidget::setRibbonShape(const std::vector<NLMISC::CVector> &verts, bool braceMode)
{
	_mode = Mode::RibbonShape;
	_braceMode = braceMode;
	_verts = verts;
	repaint();
}

void CGraphicsInfoWidget::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setBrush(QBrush(_color));
	painter.setPen(QPen(Qt::black, 2, Qt::SolidLine));
	painter.drawRoundedRect(QRect(3, 3, width() - 6, height() - 6), 3.0, 3.0);
	if (_mode == Mode::Direction)
	{
		painter.setPen(QPen(Qt::gray, 1, Qt::SolidLine));
		painter.drawLine(width() / 2, 4, width() / 2, height() - 4);
		painter.drawLine(4, height() / 2, width() - 4, height() / 2);
		painter.drawText( 10, 15, _text);

		painter.setPen(QPen(Qt::red, 2, Qt::SolidLine));
		painter.drawLine(width() / 2, height() / 2,
						 int((width() / 2) + _x * 0.9f * directionSize), int((height() / 2) - _y * 0.9f * directionSize));
	}
	if (_mode == Mode::PlaneBasic)
	{
	}
	if (_mode == Mode::RibbonShape)
	{
		painter.setPen(QPen(Qt::red, 2, Qt::SolidLine));
		painter.scale(0.86, 0.86);
		painter.translate(6, 6);
		if (_braceMode)
		{
			for(uint k = 0; k < _verts.size() / 2; ++k)
			{
				painter.drawLine(int((width() / 2.0) * (1 + _verts[2 * k].x)),
								 int((height() / 2.0) * (1 - _verts[2 * k].y)),
								 int((width() / 2.0) * (1 + _verts[2 * k + 1].x)),
								 int((height() / 2.0) * (1 - _verts[2 * k + 1].y)));
			}
		}
		else
		{
			for(uint k = 1; k < _verts.size(); k++)
			{
				painter.drawLine(int((width() / 2.0) * (1 + _verts[k - 1].x)),
								 int((height() / 2.0) * (1 - _verts[k - 1].y)),
								 int((width() / 2.0) * (1 + _verts[ k].x)),
								 int((height() / 2.0) * (1 - _verts[k].y)));
			}
			painter.drawLine(int((width() / 2.0) * (1 + _verts[0].x)),
							 int((height() / 2.0) * (1 - _verts[0].y)),
							 int((width() / 2.0) * (1 + _verts[_verts.size() - 1].x)),
							 int((height() / 2.0) * (1 - _verts[_verts.size() - 1].y)));
		}
	}
}

void CGraphicsInfoWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
	float vx = (event->x() - (width() / 2)) / 0.9f;
	float vy = ((height() / 2) - event->y()) / 0.9f;

	Q_EMIT applyNewVector(vx, vy);
}

} /* namespace NLQT */