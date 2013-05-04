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

#include "zone_painter_settings_page.h"

// Qt includes
#include <QtGui/QWidget>

// NeL includes

// Project includes

namespace Plugin
{

CZonePainterSettingsPage::CZonePainterSettingsPage(QObject *parent)
	: IOptionsPage(parent),
	  _currentPage(NULL)
{
}

QString CZonePainterSettingsPage::id() const
{
	return QLatin1String("ZonePainterPage");
}

QString CZonePainterSettingsPage::trName() const
{
	return tr("Zone Painter page");
}

QString CZonePainterSettingsPage::category() const
{
	return QLatin1String("General");
}

QString CZonePainterSettingsPage::trCategory() const
{
	return tr("General");
}

QIcon CZonePainterSettingsPage::categoryIcon() const
{
        return QIcon();
}

QWidget *CZonePainterSettingsPage::createPage(QWidget *parent)
{
	_currentPage = new QWidget(parent);
	_ui.setupUi(_currentPage);
	return _currentPage;
}

void CZonePainterSettingsPage::apply()
{
}

} /* namespace Plugin */
