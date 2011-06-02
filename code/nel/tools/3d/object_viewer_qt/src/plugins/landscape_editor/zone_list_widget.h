// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef ZONE_LIST_WIDGET_H
#define ZONE_LIST_WIDGET_H

// Project includes
#include "ui_zone_list_widget.h"

// NeL includes

// Qt includes

namespace LandscapeEditor
{
/**
@class ZoneListWidget
@brief ZoneListWidget
@details
*/
class ZoneListWidget: public QWidget
{
	Q_OBJECT

public:
	ZoneListWidget(QWidget *parent = 0);
	~ZoneListWidget();

	void setModel(QAbstractItemModel *model);

Q_SIGNALS:
public Q_SLOTS:
private Q_SLOTS:
private:
	Ui::ZoneListWidget m_ui;
}; /* ZoneListWidget */

} /* namespace LandscapeEditor */

#endif // ZONE_LIST_WIDGET_H
