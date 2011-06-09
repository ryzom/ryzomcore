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

// Project includes
#include "mission_compiler_settings_page.h"
#include "../core/core_constants.h"
#include "../core/icore.h"

// NeL includes
#include <nel/misc/path.h>

// Qt includes
#include <QtCore/QSettings>
#include <QtGui/QWidget>
#include <QtGui/QFileDialog>

namespace Plugin
{

QString lastDir = ".";

MissionCompilerSettingsPage::MissionCompilerSettingsPage(QObject *parent)
	: IOptionsPage(parent),
	  m_page(0)
{
}

MissionCompilerSettingsPage::~MissionCompilerSettingsPage()
{
}

QString MissionCompilerSettingsPage::id() const
{
		return QLatin1String("mission_compiler_settings");
}

QString MissionCompilerSettingsPage::trName() const
{
	return tr("Mission Compiler Settings");
}

QString MissionCompilerSettingsPage::category() const
{
	return QLatin1String("MissionCompilerSettings");
}

QString MissionCompilerSettingsPage::trCategory() const
{
	return tr("MissionCompilerSettings");
}

QIcon MissionCompilerSettingsPage::categoryIcon() const
{
	return QIcon();
}

QWidget *MissionCompilerSettingsPage::createPage(QWidget *parent)
{
	m_page = new QWidget(parent);
	m_ui.setupUi(m_page);

	readSettings();
	checkEnabledButton();
	connect(m_ui.addToolButton, SIGNAL(clicked()), this, SLOT(addPath()));
	connect(m_ui.removeToolButton, SIGNAL(clicked()), this, SLOT(delPath()));
	connect(m_ui.upToolButton, SIGNAL(clicked()), this, SLOT(upPath()));
	connect(m_ui.downToolButton, SIGNAL(clicked()), this, SLOT(downPath()));
	connect(m_ui.resetToolButton, SIGNAL(clicked()), m_ui.serversTreeWidget, SLOT(clear()));
	return m_page;
}

void MissionCompilerSettingsPage::apply()
{
	writeSettings();
	applySearchPaths();
}

void MissionCompilerSettingsPage::finish()
{
	delete m_page;
	m_page = 0;
}

void MissionCompilerSettingsPage::applySearchPaths()
{
	QStringList paths, remapExt;
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
	if (m_recurse)
		paths = settings->value(Core::Constants::RECURSIVE_SEARCH_PATHS).toStringList();
	else
		paths = settings->value(Core::Constants::SEARCH_PATHS).toStringList();

	remapExt = settings->value(Core::Constants::REMAP_EXTENSIONS).toStringList();
	settings->endGroup();

	for (int i = 1; i < remapExt.size(); i += 2)
		NLMISC::CPath::remapExtension(remapExt.at(i - 1).toStdString(), remapExt.at(i).toStdString(), true);

	Q_FOREACH(QString path, paths)
	{
		NLMISC::CPath::addSearchPath(path.toStdString(), m_recurse, false);
	}
}

void MissionCompilerSettingsPage::addPath()
{
	QString newPath = QFileDialog::getExistingDirectory(m_page, "", lastDir);
	if (!newPath.isEmpty())
	{
		QTreeWidgetItem *newItem = new QTreeWidgetItem;
		newItem->setText(newPath);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui.serversTreeWidget->addItem(newItem);
		lastDir = newPath;
	}

	checkEnabledButton();
}

void MissionCompilerSettingsPage::delPath()
{
	QTreeWidgetItem *removeItem = m_ui.serversTreeWidget->takeItem(m_ui.serversTreeWidget->currentRow());
	if (!removeItem)
		delete removeItem;

	checkEnabledButton();
}

void MissionCompilerSettingsPage::upPath()
{
	int currentRow = m_ui.serversTreeWidget->currentRow();
	if (!(currentRow == 0))
	{
		QListWidgetItem *item = m_ui.serversListWidget->takeItem(currentRow);
		m_ui.serversListWidget->insertItem(--currentRow, item);
		m_ui.serversListWidget->setCurrentRow(currentRow);
	}
}

void MissionCompilerSettingsPage::downPath()
{
	int currentRow = m_ui.serversListWidget->currentRow();
	if (!(currentRow == m_ui.serversListWidget->count()-1))
	{
		QListWidgetItem *item = m_ui.serversListWidget->takeItem(currentRow);
		m_ui.serversTreeWidget->insertItem(++currentRow, item);
		m_ui.serversTreeWidget->setCurrentRow(currentRow);
	}
}

void MissionCompilerSettingsPage::readSettings()
{
	QStringList paths;
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
	if (m_recurse)
		paths = settings->value(Core::Constants::RECURSIVE_SEARCH_PATHS).toStringList();
	else
		paths = settings->value(Core::Constants::SEARCH_PATHS).toStringList();
	settings->endGroup();
	Q_FOREACH(QString path, paths)
	{
		QListWidgetItem *newItem = new QListWidgetItem;
		newItem->setText(path);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui.serversTreeWidget->addItem(newItem);
	}
}

void MissionCompilerSettingsPage::writeSettings()
{
	QStringList paths;
	for (int i = 0; i < m_ui.serversTreeWidget->count(); ++i)
		paths << m_ui.serversTreeWidget->item(i)->text();

	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
	if (m_recurse)
		settings->setValue(Core::Constants::RECURSIVE_SEARCH_PATHS, paths);
	else
		settings->setValue(Core::Constants::SEARCH_PATHS, paths);
	settings->endGroup();
	settings->sync();
}

void MissionCompilerSettingsPage::checkEnabledButton()
{
	bool bEnabled = true;
	if (m_ui.serversTreeWidget->count() == 0)
		bEnabled = false;

	m_ui.removeToolButton->setEnabled(bEnabled);
	m_ui.upToolButton->setEnabled(bEnabled);
	m_ui.downToolButton->setEnabled(bEnabled);
}

} /* namespace Plugin */