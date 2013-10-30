// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

// Project includes
#include "world_editor_settings_page.h"
#include "world_editor_constants.h"

// Qt includes
#include <QtGui/QWidget>

// NeL includes

namespace WorldEditor
{

WorldEditorSettingsPage::WorldEditorSettingsPage(QObject *parent)
	: IOptionsPage(parent),
	  m_currentPage(NULL)
{
}

QString WorldEditorSettingsPage::id() const
{
	return QLatin1String(Constants::WORLD_EDITOR_PLUGIN);
}

QString WorldEditorSettingsPage::trName() const
{
	return tr("General");
}

QString WorldEditorSettingsPage::category() const
{
	return QLatin1String(Constants::WORLD_EDITOR_PLUGIN);
}

QString WorldEditorSettingsPage::trCategory() const
{
	return tr("World Editor");
}

QIcon WorldEditorSettingsPage::categoryIcon() const
{
	return QIcon();
}

QWidget *WorldEditorSettingsPage::createPage(QWidget *parent)
{
	m_currentPage = new QWidget(parent);
	m_ui.setupUi(m_currentPage);
	return m_currentPage;
}

void WorldEditorSettingsPage::apply()
{
}

} /* namespace WorldEditor */