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
#include "search_paths_settings_page.h"
#include "core_constants.h"
#include "icore.h"

// NeL includes
#include <nel/misc/path.h>

// Qt includes
#include <QtCore/QSettings>
#include <QtGui/QWidget>
#include <QtGui/QFileDialog>

namespace Core
{

QString lastDir = ".";

CSearchPathsSettingsPage::CSearchPathsSettingsPage(bool recurse, QObject *parent)
	: IOptionsPage(parent),
	  m_recurse(recurse),
	  m_page(0)
{
}

CSearchPathsSettingsPage::~CSearchPathsSettingsPage()
{
}

QString CSearchPathsSettingsPage::id() const
{
	if (m_recurse)
		return QLatin1String("search_recurse_paths");
	else
		return QLatin1String("search_paths");
}

QString CSearchPathsSettingsPage::trName() const
{
	if (m_recurse)
		return tr("Search Recurse Paths");
	else
		return tr("Search Paths");
}

QString CSearchPathsSettingsPage::category() const
{
	return QLatin1String(Constants::SETTINGS_CATEGORY_GENERAL);
}

QString CSearchPathsSettingsPage::trCategory() const
{
	return tr(Constants::SETTINGS_TR_CATEGORY_GENERAL);
}

QWidget *CSearchPathsSettingsPage::createPage(QWidget *parent)
{
	m_page = new QWidget(parent);
	m_ui.setupUi(m_page);

	readSettings();
	checkEnabledButton();
	connect(m_ui.addToolButton, SIGNAL(clicked()), this, SLOT(addPath()));
	connect(m_ui.removeToolButton, SIGNAL(clicked()), this, SLOT(delPath()));
	connect(m_ui.upToolButton, SIGNAL(clicked()), this, SLOT(upPath()));
	connect(m_ui.downToolButton, SIGNAL(clicked()), this, SLOT(downPath()));
	connect(m_ui.resetToolButton, SIGNAL(clicked()), m_ui.pathsListWidget, SLOT(clear()));
	return m_page;
}

void CSearchPathsSettingsPage::apply()
{
	writeSettings();
	applySearchPaths();
}

void CSearchPathsSettingsPage::finish()
{
	delete m_page;
	m_page = 0;
}

void CSearchPathsSettingsPage::applySearchPaths()
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

void CSearchPathsSettingsPage::addPath()
{
	QString newPath = QFileDialog::getExistingDirectory(m_page, "", lastDir);
	if (!newPath.isEmpty())
	{
		QListWidgetItem *newItem = new QListWidgetItem;
		newItem->setText(newPath);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui.pathsListWidget->addItem(newItem);
		lastDir = newPath;
	}

	checkEnabledButton();
}

void CSearchPathsSettingsPage::delPath()
{
	QListWidgetItem *removeItem = m_ui.pathsListWidget->takeItem(m_ui.pathsListWidget->currentRow());
	if (!removeItem)
		delete removeItem;

	checkEnabledButton();
}

void CSearchPathsSettingsPage::upPath()
{
	int currentRow = m_ui.pathsListWidget->currentRow();
	if (!(currentRow == 0))
	{
		QListWidgetItem *item = m_ui.pathsListWidget->takeItem(currentRow);
		m_ui.pathsListWidget->insertItem(--currentRow, item);
		m_ui.pathsListWidget->setCurrentRow(currentRow);
	}
}

void CSearchPathsSettingsPage::downPath()
{
	int currentRow = m_ui.pathsListWidget->currentRow();
	if (!(currentRow == m_ui.pathsListWidget->count()-1))
	{
		QListWidgetItem *item = m_ui.pathsListWidget->takeItem(currentRow);
		m_ui.pathsListWidget->insertItem(++currentRow, item);
		m_ui.pathsListWidget->setCurrentRow(currentRow);
	}
}

void CSearchPathsSettingsPage::readSettings()
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
		m_ui.pathsListWidget->addItem(newItem);
	}
}

void CSearchPathsSettingsPage::writeSettings()
{
	QStringList paths;
	for (int i = 0; i < m_ui.pathsListWidget->count(); ++i)
		paths << m_ui.pathsListWidget->item(i)->text();

	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
	if (m_recurse)
		paths = settings->value(Core::Constants::RECURSIVE_SEARCH_PATHS).toStringList();
	else
		paths = settings->value(Core::Constants::SEARCH_PATHS).toStringList();
	settings->endGroup();
}

void CSearchPathsSettingsPage::checkEnabledButton()
{
	bool bEnabled = true;
	if (m_ui.pathsListWidget->count() == 0)
		bEnabled = false;

	m_ui.removeToolButton->setEnabled(bEnabled);
	m_ui.upToolButton->setEnabled(bEnabled);
	m_ui.downToolButton->setEnabled(bEnabled);
}

} /* namespace Core */