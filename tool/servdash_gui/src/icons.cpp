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

#include <QPainter>
#include <QPainterPath>

namespace {

/// The blue of the original control icons.
const QColor s_ControlBlue(0x3A, 0x8A, 0xE0);

QPixmap paintControl(int size, bool play)
{
	QPixmap pixmap(size, size);
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(QPen(s_ControlBlue.darker(135), size / 16.0));
	painter.setBrush(s_ControlBlue);
	qreal m = size * 0.2;
	if (play)
	{
		QPainterPath path;
		path.moveTo(m + size * 0.06, m);
		path.lineTo(size - m + size * 0.04, size / 2.0);
		path.lineTo(m + size * 0.06, size - m);
		path.closeSubpath();
		painter.drawPath(path);
	}
	else
	{
		painter.drawRoundedRect(QRectF(m, m, size - 2 * m, size - 2 * m), size * 0.08, size * 0.08);
	}
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
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(QPen(color.darker(150), 1));
	painter.setBrush(color);
	painter.drawEllipse(QRectF(1.5, 1.5, size - 3, size - 3));
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
