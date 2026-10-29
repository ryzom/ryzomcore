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

#ifndef SERVDASH_DASHBOARD_STYLE_H
#define SERVDASH_DASHBOARD_STYLE_H

#include <QProxyStyle>

/// Fusion, with scrollbars painted from filled rectangles. Fusion strokes their outlines as
/// one pixel lines, which a fractional display scale drops, so the bars looked cut open on
/// every side at 125 percent. The look stays Fusion's: a shaded groove, a bevelled handle and
/// the arrow buttons, with the arrows drawn by Fusion itself. Colours come from the palette,
/// so a dark theme is covered.
class CDashboardStyle : public QProxyStyle
{
	Q_OBJECT

public:
	explicit CDashboardStyle(QStyle *base);

	void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = nullptr) const override;

private:
	void drawBevel(QPainter *painter, const QRect &rect, const QPalette &palette, bool hover, bool sunken) const;
};

#endif // SERVDASH_DASHBOARD_STYLE_H
