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

#include "server_entry_dialog.h"

// NeL includes
#include <nel/misc/path.h>

// Qt includes
#include <QtCore/QSettings>
#include <QtGui/QWidget>
#include <QtGui/QFileDialog>
#include <QtGui/QTreeWidgetItem>

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
	return QLatin1String("Mission Compiler");
}

QString MissionCompilerSettingsPage::trCategory() const
{
	return tr("Mission Compiler");
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
	connect(m_ui.addToolButton, SIGNAL(clicked()), this, SLOT(addServer()));
	connect(m_ui.removeToolButton, SIGNAL(clicked()), this, SLOT(delServer()));
	connect(m_ui.serversTableWidget, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(editServer(int,int)));
	return m_page;
}

void MissionCompilerSettingsPage::apply()
{
	writeSettings();
}

void MissionCompilerSettingsPage::finish()
{
	delete m_page;
	m_page = 0;
}

void MissionCompilerSettingsPage::editServer(int row, int column)
{
	ServerEntryDialog serverEntryDialog;
	serverEntryDialog.setModal(true);
	serverEntryDialog.show();

	// Copy the values from the row to the dialog.
	QTableWidgetItem *item1 = m_ui.serversTableWidget->item(row,0);
	QTableWidgetItem *item2 = m_ui.serversTableWidget->item(row,1);
	QTableWidgetItem *item3 = m_ui.serversTableWidget->item(row,2);
	serverEntryDialog.setServerName(item1->text());
	serverEntryDialog.setTextPath(item2->text());
	serverEntryDialog.setPrimPath(item3->text());

	if(serverEntryDialog.exec())
	{
		item1->setText(serverEntryDialog.getServerName());
		item2->setText(serverEntryDialog.getTextPath());
		item3->setText(serverEntryDialog.getPrimPath());
	}
}

void MissionCompilerSettingsPage::addServer()
{
	ServerEntryDialog serverEntryDialog;
	serverEntryDialog.setModal(true);
	serverEntryDialog.show();
	

	if(serverEntryDialog.exec())
	{
		int row = m_ui.serversTableWidget->rowCount();
		m_ui.serversTableWidget->insertRow(row);
		QTableWidgetItem *item1 = new QTableWidgetItem(serverEntryDialog.getServerName());
		QTableWidgetItem *item2 = new QTableWidgetItem(serverEntryDialog.getTextPath());
		QTableWidgetItem *item3 = new QTableWidgetItem(serverEntryDialog.getPrimPath());

		m_ui.serversTableWidget->setItem(row, 0, item1);
		m_ui.serversTableWidget->setItem(row, 1, item2);
		m_ui.serversTableWidget->setItem(row, 2, item3);
	}
}

void MissionCompilerSettingsPage::delServer()
{
	QList<QTableWidgetItem*> selectedItems = m_ui.serversTableWidget->selectedItems();
	while(selectedItems.size() > 0)
	{
		m_ui.serversTableWidget->removeRow(selectedItems.back()->row());
		selectedItems = m_ui.serversTableWidget->selectedItems();
	}
}

void MissionCompilerSettingsPage::readSettings()
{
	//QStringList paths;
	//QSettings *settings = Core::ICore::instance()->settings();
	//settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
	//if (m_recurse)
	//	paths = settings->value(Core::Constants::RECURSIVE_SEARCH_PATHS).toStringList();
	//else
	//	paths = settings->value(Core::Constants::SEARCH_PATHS).toStringList();
	//settings->endGroup();
	//Q_FOREACH(QString path, paths)
	//{
	//	QListWidgetItem *newItem = new QListWidgetItem;
	//	newItem->setText(path);
	//	newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	//	m_ui.serversTreeWidget->addItem(newItem);
	//}
}

void MissionCompilerSettingsPage::writeSettings()
{
	//QStringList paths;
	//for (int i = 0; i < m_ui.serversTreeWidget->count(); ++i)
	//	paths << m_ui.serversTreeWidget->item(i)->text();

	//QSettings *settings = Core::ICore::instance()->settings();
	//settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
	//if (m_recurse)
	//	settings->setValue(Core::Constants::RECURSIVE_SEARCH_PATHS, paths);
	//else
	//	settings->setValue(Core::Constants::SEARCH_PATHS, paths);
	//settings->endGroup();
	//settings->sync();
}

} /* namespace Plugin */