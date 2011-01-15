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


#ifndef SEARCH_PATHS_SETTINGS_PAGE_H
#define SEARCH_PATHS_SETTINGS_PAGE_H

#include <QtCore/QObject>

#include "ioptions_page.h"

#include "ui_search_paths_settings_page.h"

class QWidget;

namespace Core
{
/**
@class CSearchPathsSettingsPage
*/
class CSearchPathsSettingsPage : public IOptionsPage
{
	Q_OBJECT
public:
	CSearchPathsSettingsPage(QObject *parent = 0);
	virtual ~CSearchPathsSettingsPage() {}

	virtual QString id() const;
	virtual QString trName() const;
	virtual QString category() const;
	virtual QString trCategory() const;
	virtual QWidget *createPage(QWidget *parent);

	virtual void apply();
	virtual void finish() {}

private:
	QWidget *_currentPage;
	Ui::CSearchPathsSettingsPage _ui;
};

} // namespace Core

#endif // SEARCH_PATHS_SETTINGS_H
