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
#include "mission_compiler_plugin_constants.h"
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

namespace MissionCompiler
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
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::MISSION_COMPILER_SECTION);

	// Retrieve the local text path.
	m_ui.localPathEdit->setText(settings->value(Constants::SETTING_LOCAL_TEXT_PATH).toString());

	QStringList items = settings->value(Constants::SETTING_SERVERS_TABLE_ITEMS).toStringList();
	int column = 0;
	int row = 0;
	m_ui.serversTableWidget->insertRow(row);
	Q_FOREACH(QString var, items)
	{
		// Check to see if we're starting a new row.
		if(column > 2)
		{
			column = 0;
			row++;
			m_ui.serversTableWidget->insertRow(row);
		}
		
		QTableWidgetItem *item = new QTableWidgetItem(var);
		m_ui.serversTableWidget->setItem(row, column, item);

		column++;
	}
	settings->endGroup();
}

void MissionCompilerSettingsPage::writeSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::MISSION_COMPILER_SECTION);
	
	// Save the local text path.
	settings->setValue(Constants::SETTING_LOCAL_TEXT_PATH, m_ui.localPathEdit->text());

	QStringList items;
	for(int row = 0; row < m_ui.serversTableWidget->rowCount(); row++)
	{
		for(int column = 0; column < m_ui.serversTableWidget->columnCount(); column++)
		{
			items << m_ui.serversTableWidget->item(row, column)->text();
		}
	}

	settings->setValue(Constants::SETTING_SERVERS_TABLE_ITEMS, items);
	settings->endGroup();
	settings->sync();
}

} /* namespace MissionCompiler */