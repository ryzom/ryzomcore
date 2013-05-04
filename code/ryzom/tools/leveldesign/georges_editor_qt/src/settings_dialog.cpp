/*
Georges Editor Qt
Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "settings_dialog.h"

// Qt includes
#include <QtGui/QWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

// NeL includes
#include <nel/misc/path.h>

// Project includes
#include "modules.h"

using namespace NLMISC;

namespace NLQT 
{

	CSettingsDialog::CSettingsDialog(QWidget *parent)
		: QDialog(parent)
	{
		ui.setupUi(this);

		while (ui.driverGraphComboBox->count())
			ui.driverGraphComboBox->removeItem(0);

		// load types graphics driver from the config file
		NLMISC::CConfigFile::CVar v = Modules::config().getConfigFile().getVar("GraphicsDrivers");
		for (uint i = 0; i < v.size(); ++i)
			ui.driverGraphComboBox->addItem(v.asString(i).c_str());

		// set graphics driver from the config file
		QString value = Modules::config().getValue("GraphicsDriver",std::string("OpenGL")).c_str();
		QString dn = value.toLower();
		for (sint i = 0; i < ui.driverGraphComboBox->count(); ++i)
		{
			if (dn == ui.driverGraphComboBox->itemText(i).toLower())
			{
				ui.driverGraphComboBox->setCurrentIndex(i);
			}
		}

		// load leveldesign path from the config file
		NLMISC::CConfigFile::CVar v2 = Modules::config().getConfigFile().getVar("LeveldesignPath");
		ui.leveldesignPath->setText(v2.asString().c_str());

		// load search paths from the config file
		NLMISC::CConfigFile::CVar v3 = Modules::config().getConfigFile().getVar("SearchPaths");
		ui.pathsListWidget->clear();

		for (uint i = 0; i < v3.size(); ++i)
		{
			ui.pathsListWidget->addItem(v3.asString(i).c_str());
			ui.pathsListWidget->item(i)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		}

		connect(ui.addToolButton, SIGNAL(clicked()), this, SLOT(addPath()));
		connect(ui.removeToolButton, SIGNAL(clicked()), this, SLOT(removePath()));
		connect(ui.upToolButton, SIGNAL(clicked()), this, SLOT(upPath()));
		connect(ui.downToolButton, SIGNAL(clicked()), this, SLOT(downPath()));
		connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(applyPressed()));

		connect(ui.browseLdPath, SIGNAL(clicked()), this, SLOT(browseLeveldesignPath()));

#ifdef NL_OS_UNIX
		ui.driverGraphComboBox->setEnabled(false);
#endif

	}

	CSettingsDialog::~CSettingsDialog()
	{
	}

	void CSettingsDialog::addPath()
	{
		QListWidgetItem *newItem = new QListWidgetItem;
		QFileDialog dialog(this);
		dialog.setOption(QFileDialog::ShowDirsOnly, true);
		dialog.setFileMode(QFileDialog::Directory);
		if (dialog.exec())
		{
			QString newPath = dialog.selectedFiles().first();
			if (!newPath.isEmpty()) 
			{
				newItem->setText(newPath);
				newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				ui.pathsListWidget->addItem(newItem);
			}
		}
	}

	void CSettingsDialog::removePath()
	{
		QListWidgetItem *removeItem = ui.pathsListWidget->takeItem(ui.pathsListWidget->currentRow());
		if (!removeItem) delete removeItem;
	}

	void CSettingsDialog::upPath()
	{
		sint currentRow = ui.pathsListWidget->currentRow();
		if (!(currentRow == 0))
		{
			QListWidgetItem *item = ui.pathsListWidget->takeItem(currentRow);
			ui.pathsListWidget->insertItem(--currentRow, item);  
			ui.pathsListWidget->setCurrentRow(currentRow);
		}
	}

	void CSettingsDialog::downPath()
	{
		sint currentRow = ui.pathsListWidget->currentRow();
		if (!(currentRow == ui.pathsListWidget->count()-1))
		{
			QListWidgetItem *item = ui.pathsListWidget->takeItem(currentRow);
			ui.pathsListWidget->insertItem(++currentRow, item);  
			ui.pathsListWidget->setCurrentRow(currentRow);
		}
	}

	void CSettingsDialog::applyPressed()
	{

		// settings take after restart the program
		/*QMessageBox::warning(this, tr("Settings"), 
		tr("Graphics and sound settings "
		"take after restart the program"),
		QMessageBox::Ok);*/

		// save graphics settings to config file
		Modules::config().getConfigFile().getVar("GraphicsDriver").setAsString(ui.driverGraphComboBox->currentText().toUtf8().constData());

		// save leveldesign path to config file
		QString oldLdPath = Modules::config().getValue("LeveldesignPath", std::string("")).c_str();
		if (oldLdPath != ui.leveldesignPath->text())
		{
			std::string ldPath = ui.leveldesignPath->text().toUtf8().constData();
			Modules::config().getConfigFile().getVar("LeveldesignPath").forceAsString(ldPath);
			Q_EMIT ldPathChanged(ldPath.c_str());
			// TODO: remove old Path from CPath
			Modules::config().addLeveldesignPath();
		}

		// save search paths to config file
		NLMISC::CConfigFile::CVar v = Modules::config().getConfigFile().getVar("SearchPaths");
		QStringList sl;
		for (uint i = 0; i < v.size(); ++i)
		{
			sl.append(v.asString(i).c_str());
		}

		std::vector<std::string> list;
		std::vector<std::string> addList;
		for (sint i = 0; i < ui.pathsListWidget->count(); ++i)
		{
			std::string str = ui.pathsListWidget->item(i)->text().toUtf8().constData();
			if (!str.empty())
			{
				list.push_back(str);
				if (!sl.contains(str.c_str()))
				{
					addList.push_back(str);
				}
			}
		}

		if (list.empty()) 
		{
			Modules::config().getConfigFile().getVar("SearchPaths").forceAsString("");
		}
		else 
		{
			Modules::config().getConfigFile().getVar("SearchPaths").forceAsString("");
			Modules::config().getConfigFile().getVar("SearchPaths").setAsString(list);
		}

		// save config file
		Modules::config().getConfigFile().save();

		// reload search paths
		Modules::config().addSearchPaths(&addList);
	}

	void CSettingsDialog::browseLeveldesignPath()
	{
		ui.leveldesignPath->setText(QFileDialog::getExistingDirectory(this, tr("Open Directory"),
			QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
	}

} /* namespace NLQT */
