// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef TILE_WIDGET_H
#define TILE_WIDGET_H

#include <QListWidgetItem>
#include "ui_tile_widget_qt.h"

class CTile_ListWidgetItem : public QListWidgetItem
{
public:
	CTile_ListWidgetItem ( QListWidget * parent, int type = Type ):QListWidgetItem(parent,type){}

	CTile_ListWidgetItem(QWidget *parent = 0);
	void initWidget(const QPixmap&, const QString&);

private:
	Ui::TileWidget ui;
	// Qpixmap tilePixmap;
	// QString tileLabel;
};

#endif