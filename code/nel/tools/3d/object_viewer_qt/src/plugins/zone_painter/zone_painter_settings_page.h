// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


#ifndef ZONE_PAINTER_SETTINGS_PAGE_H
#define ZONE_PAINTER_SETTINGS_PAGE_H

#include <QtCore/QObject>

#include "../core/ioptions_page.h"

#include "ui_zone_painter_settings_page.h"

class QWidget;

namespace Plugin
{
/**
@class CZonePainterSettingsPage
*/
class CZonePainterSettingsPage : public Core::IOptionsPage
{
	Q_OBJECT
public:
	CZonePainterSettingsPage(QObject *parent = 0);
	virtual ~CZonePainterSettingsPage() {}

	virtual QString id() const;
	virtual QString trName() const;
	virtual QString category() const;
	virtual QString trCategory() const;
	QIcon categoryIcon() const;
	virtual QWidget *createPage(QWidget *parent);

	virtual void apply();
	virtual void finish() {}

private:
	QWidget *_currentPage;
	Ui::CZonePainterSettingsPage _ui;
};

} // namespace Plugin

#endif // EXAMPLE_SETTINGS_H
