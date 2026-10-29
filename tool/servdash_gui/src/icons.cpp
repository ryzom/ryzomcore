// Ryzom Core - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "icons.h"

#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPen>

namespace {

/// The blue of the Fugue control icons the original used.
const QColor s_ControlBlue(0x43, 0x91, 0xCD);

/// A shade of a base colour: lightness and saturation moved by the given percentages.
QColor shade(const QColor &base, int lightness, int saturation = 0)
{
	int h, s, l, a;
	base.getHsl(&h, &s, &l, &a);
	s = qBound(0, s + saturation * 255 / 100, 255);
	l = qBound(0, l + lightness * 255 / 100, 255);
	return QColor::fromHsl(h, s, l, a);
}

/// Paints a shape the way the Fugue icons are drawn: a glossy fill that is pale at the top,
/// a shade darker through the middle and bright and saturated at the bottom; a one pixel
/// highlight ring just inside the edge, light at the top and fading to the bright bottom
/// colour; and a one pixel outer edge that runs from the base colour at the top to a dark
/// shade at the bottom. That is what keeps the small icons sharp against the background.
void paintGlossy(QPainter &painter, const QPainterPath &shape, const QColor &base, qreal unit)
{
	const QRectF bounds = shape.boundingRect();
	const QPointF top(bounds.center().x(), bounds.top());
	const QPointF bottom(bounds.center().x(), bounds.bottom());

	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(Qt::NoPen);

	QLinearGradient fill(top, bottom);
	fill.setColorAt(0.0, shade(base, 15));
	fill.setColorAt(0.45, shade(base, -2));
	fill.setColorAt(0.55, shade(base, 0, 15));
	fill.setColorAt(1.0, shade(base, 8, 20));
	painter.setBrush(fill);
	painter.drawPath(shape);

	// the inner highlight ring
	QPainterPathStroker inner;
	inner.setWidth(unit);
	QLinearGradient ring(top, bottom);
	ring.setColorAt(0.0, shade(base, 28, 10));
	ring.setColorAt(0.5, shade(base, 14, 10));
	ring.setColorAt(1.0, shade(base, 10, 25));
	painter.setBrush(ring);
	// the ring sits one unit inside the edge; stroking the shape inset by that unit gives it
	QPainterPathStroker insetStroker;
	insetStroker.setWidth(unit * 2);
	QPainterPath insetArea = shape.subtracted(insetStroker.createStroke(shape));
	painter.drawPath(inner.createStroke(insetArea).intersected(shape));

	// the outer edge
	QPainterPathStroker edge;
	edge.setWidth(unit);
	QLinearGradient edgeColor(top, bottom);
	QColor edgeTop = shade(base, 2);
	edgeTop.setAlphaF(0.85);
	QColor edgeBottom = shade(base, -22);
	edgeBottom.setAlphaF(0.9);
	edgeColor.setColorAt(0.0, edgeTop);
	edgeColor.setColorAt(1.0, edgeBottom);
	painter.setBrush(edgeColor);
	painter.drawPath(edge.createStroke(shape).intersected(shape));
}

QPixmap paintControl(int size, bool play)
{
	QPixmap pixmap(size, size);
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	const qreal unit = size / 16.0;
	QPainterPath shape;
	if (play)
	{
		// the Fugue play: a triangle from the left quarter to the right edge, corners cut
		const qreal left = 4.0 * unit, right = 12.5 * unit, top = 1.5 * unit, bottom = 14.5 * unit, cut = 0.8 * unit;
		shape.moveTo(left, top + cut);
		shape.lineTo(left + cut, top);
		shape.lineTo(right, (top + bottom) / 2 - cut * 0.5);
		shape.lineTo(right, (top + bottom) / 2 + cut * 0.5);
		shape.lineTo(left + cut, bottom);
		shape.lineTo(left, bottom - cut);
		shape.closeSubpath();
	}
	else
	{
		shape.addRoundedRect(QRectF(2.5 * unit, 2.5 * unit, 11 * unit, 11 * unit), unit, unit);
	}
	paintGlossy(painter, shape, s_ControlBlue, unit);
	return pixmap;
}

QIcon controlIcon(bool play)
{
	QIcon icon;
	icon.addPixmap(paintControl(16, play));
	icon.addPixmap(paintControl(24, play));
	icon.addPixmap(paintControl(32, play));
	icon.addPixmap(paintControl(48, play));
	return icon;
}

} // namespace

namespace Icons {

QIcon play()
{
	static const QIcon s_Icon = controlIcon(true);
	return s_Icon;
}

QIcon stop()
{
	static const QIcon s_Icon = controlIcon(false);
	return s_Icon;
}

QPixmap light(const QColor &color, int size)
{
	QPixmap pixmap(size, size);
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	const qreal unit = size / 11.0;
	QPainterPath shape;
	shape.addEllipse(QRectF(0.5 * unit, 0.5 * unit, size - unit, size - unit));
	paintGlossy(painter, shape, color, unit);
	return pixmap;
}

QColor logColor(const QColor &color, const QPalette &palette)
{
	if (!color.isValid())
		return palette.color(QPalette::Text);
	bool dark = palette.color(QPalette::Base).lightness() < 128;
	if (!dark)
		return color;
	// a colour that would sink into a dark background is brightened, keeping its hue
	if (color.lightness() < 110)
	{
		QColor bright = color.toHsl();
		bright.setHsl(bright.hue(), bright.saturation(), 160);
		return bright.toRgb();
	}
	return color;
}

} // namespace Icons
