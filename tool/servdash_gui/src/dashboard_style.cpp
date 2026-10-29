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

#include "dashboard_style.h"

#include <QLinearGradient>
#include <QPainter>
#include <QStyleOptionSlider>

CDashboardStyle::CDashboardStyle(QStyle *base)
    : QProxyStyle(base)
{
}

void CDashboardStyle::drawBevel(QPainter *painter, const QRect &rect, const QPalette &palette, bool hover, bool sunken) const
{
	if (rect.width() < 4 || rect.height() < 4)
		return;
	const QColor outline = palette.color(QPalette::Window).darker(140);
	QColor button = palette.color(QPalette::Button);
	if (sunken)
		button = button.darker(110);
	else if (hover)
		button = button.lighter(104);

	// a one pixel inset keeps the bevel off the groove edge, as Fusion does
	const QRect r = rect.adjusted(1, 1, -1, -1);
	QLinearGradient gradient(r.topLeft(), rect.width() > rect.height() ? r.bottomLeft() : r.topRight());
	gradient.setColorAt(0, button.lighter(112));
	gradient.setColorAt(1, button.darker(102));
	painter->fillRect(r, gradient);

	// the outline from four fills, with the corner pixels left out for a rounded look
	painter->fillRect(QRect(r.left() + 1, r.top(), r.width() - 2, 1), outline);
	painter->fillRect(QRect(r.left() + 1, r.bottom(), r.width() - 2, 1), outline);
	painter->fillRect(QRect(r.left(), r.top() + 1, 1, r.height() - 2), outline);
	painter->fillRect(QRect(r.right(), r.top() + 1, 1, r.height() - 2), outline);
	// the inner highlight along the top and the left
	const QColor highlight = button.lighter(125);
	painter->fillRect(QRect(r.left() + 1, r.top() + 1, r.width() - 2, 1), highlight);
	painter->fillRect(QRect(r.left() + 1, r.top() + 1, 1, r.height() - 2), highlight);
}

void CDashboardStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
	const QStyleOptionSlider *bar = control == CC_ScrollBar ? qstyleoption_cast<const QStyleOptionSlider *>(option) : nullptr;
	if (bar == nullptr)
	{
		QProxyStyle::drawComplexControl(control, option, painter, widget);
		return;
	}

	const QPalette &palette = bar->palette;
	const bool horizontal = bar->orientation == Qt::Horizontal;
	const QColor window = palette.color(QPalette::Window);
	const QColor outline = window.darker(140);
	const QRect groove = subControlRect(CC_ScrollBar, bar, SC_ScrollBarGroove, widget);
	const QRect slider = subControlRect(CC_ScrollBar, bar, SC_ScrollBarSlider, widget);
	const QRect subLine = subControlRect(CC_ScrollBar, bar, SC_ScrollBarSubLine, widget);
	const QRect addLine = subControlRect(CC_ScrollBar, bar, SC_ScrollBarAddLine, widget);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, false);

	// the groove, shaded towards the viewport
	painter->fillRect(bar->rect, window);
	QLinearGradient shade(groove.topLeft(), horizontal ? groove.bottomLeft() : groove.topRight());
	shade.setColorAt(0, window.darker(114));
	shade.setColorAt(1, window.darker(104));
	painter->fillRect(groove, shade);
	// the edge towards the viewport
	if (horizontal)
		painter->fillRect(QRect(bar->rect.left(), bar->rect.top(), bar->rect.width(), 1), outline);
	else
		painter->fillRect(QRect(bar->rect.left(), bar->rect.top(), 1, bar->rect.height()), outline);

	const bool enabled = bar->state & State_Enabled;
	const bool hover = enabled && (bar->state & State_MouseOver);
	const bool sunken = bar->state & State_Sunken;

	if (bar->subControls & SC_ScrollBarSlider)
		drawBevel(painter, slider, palette, hover && (bar->activeSubControls & SC_ScrollBarSlider), sunken && (bar->activeSubControls & SC_ScrollBarSlider));

	// the arrow buttons, with Fusion's own arrows
	const struct
	{
		SubControl Control;
		QRect Rect;
		PrimitiveElement Arrow;
	} buttons[2] = {
		{ SC_ScrollBarSubLine, subLine, horizontal ? PE_IndicatorArrowLeft : PE_IndicatorArrowUp },
		{ SC_ScrollBarAddLine, addLine, horizontal ? PE_IndicatorArrowRight : PE_IndicatorArrowDown },
	};
	for (int i = 0; i < 2; ++i)
	{
		if (!(bar->subControls & buttons[i].Control))
			continue;
		const bool active = bar->activeSubControls & buttons[i].Control;
		drawBevel(painter, buttons[i].Rect, palette, hover && active, sunken && active);
		QStyleOption arrow = *bar;
		arrow.rect = buttons[i].Rect.adjusted(2, 2, -2, -2);
		arrow.state = bar->state & (State_Enabled | State_Active);
		QProxyStyle::drawPrimitive(buttons[i].Arrow, &arrow, painter, widget);
	}

	painter->restore();
}
