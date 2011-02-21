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

#include "log_settings_page.h"

// Qt includes
#include <QtGui/QWidget>

// NeL includes

// Project includes
namespace ExtensionSystem
{
	class IPluginManager;
}

namespace Plugin
{

	CLogSettingsPage::CLogSettingsPage(QObject *parent)
		: IOptionsPage(parent),
		_currentPage(NULL)
	{
	}

	QString CLogSettingsPage::id() const
	{
		return QLatin1String("Log");
	}

	QString CLogSettingsPage::trName() const
	{
		return tr("Log");
	}

	QString CLogSettingsPage::category() const
	{
		return QLatin1String("General");
	}

	QString CLogSettingsPage::trCategory() const
	{
		return tr("General");
	}

	QWidget *CLogSettingsPage::createPage(QWidget *parent)
	{
		_currentPage = new QWidget(parent);
		_ui.setupUi(_currentPage);
		return _currentPage;
	}

	void CLogSettingsPage::apply()
	{
		//ExtensionSystem::IPluginSpec *spec, _plugMan->plugins()
		//ExtensionSystem::IPluginManager;
		//if (_ui.errorCheck->isChecked()) {
			//displayer();
		//}
		//if (_ui.warningCheck->isChecked());
		//if (_ui.debugCheck->isChecked());
		//if (_ui.assertCheck->isChecked());
		//if (_ui.infoCheck->isChecked());
	}

} /* namespace Plugin */