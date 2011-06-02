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

#ifndef LIST_ZONES_WIDGET_H
#define LIST_ZONES_WIDGET_H

// Project includes
#include "ui_list_zones_widget.h"

// NeL includes

// Qt includes

namespace LandscapeEditor
{
class ZoneBuilder;

/**
@class ZoneListWidget
@brief ZoneListWidget
@details
*/
class ListZonesWidget: public QWidget
{
	Q_OBJECT

public:
	ListZonesWidget(QWidget *parent = 0);
	~ListZonesWidget();

	void updateUi();
	void setZoneBuilder(ZoneBuilder *zoneBuilder);
	void setModel(QAbstractItemModel *model);

Q_SIGNALS:
public Q_SLOTS:
	void updateFilters_1(const QString &value);
	void updateFilters_2(const QString &value);
	void updateFilters_3(const QString &value);
	void updateFilters_4(const QString &value);

private Q_SLOTS:
	void updateListZones();

private:
	void disableSignals(bool block);

	ZoneBuilder *m_zoneBuilder;
	Ui::ListZonesWidget m_ui;
}; /* ZoneListWidget */

} /* namespace LandscapeEditor */

#endif // LIST_ZONES_WIDGET_H
