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

#ifndef SERVDASH_ICONS_H
#define SERVDASH_ICONS_H

#include <QColor>
#include <QIcon>
#include <QPalette>
#include <QPixmap>

/// The few pictures of the dashboard, painted at runtime so they scale with the screen and
/// read on a light or a dark theme: the play and stop buttons in the blue of the original
/// icons, and the state lights.
namespace Icons {

QIcon play();
QIcon stop();
QPixmap light(const QColor &color, int size);

/// A console log colour as the NeL window used it, adjusted for a dark theme where the
/// dark red of warnings and the black of plain lines would vanish.
QColor logColor(const QColor &color, const QPalette &palette);

} // namespace Icons

#endif // SERVDASH_ICONS_H
