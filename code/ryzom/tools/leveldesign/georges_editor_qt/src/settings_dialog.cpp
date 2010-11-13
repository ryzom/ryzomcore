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

		// setup config file callbacks and initialize values
		Modules::config().setAndCallback("GraphicsDrivers", CConfigCallback(this, &CSettingsDialog::cfcbGraphicsDrivers));
		Modules::config().setAndCallback("SearchPaths", CConfigCallback(this, &CSettingsDialog::cfcbSearchPaths));
		Modules::config().setAndCallback("LeveldesignPath", CConfigCallback(this, &CSettingsDialog::cfcbLeveldesignPath));

		// load settings from the config file

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
		Modules::config().dropCallback("GraphicsDrivers");
		Modules::config().dropCallback("SearchPaths");
		Modules::config().dropCallback("LeveldesignPath");
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
		Modules::config().getConfigFile().getVar("GraphicsDriver").setAsString(ui.driverGraphComboBox->currentText().toStdString());

		// save leveldesign path to config file
		std::string ldPath = ui.leveldesignPath->text().toStdString();
		Modules::config().getConfigFile().getVar("LeveldesignPath").forceAsString(ldPath);
		Q_EMIT ldPathChanged(ldPath.c_str());

		// save search paths to config file
		std::vector<std::string> list;
		for (sint i = 0; i < ui.pathsListWidget->count(); ++i)
		{
			std::string str = ui.pathsListWidget->item(i)->text().toStdString();
			if (str != "")
				list.push_back(str);
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
		Modules::config().configSearchPaths();
		Modules::config().configLeveldesignPath();
	}

	void CSettingsDialog::cfcbGraphicsDrivers(NLMISC::CConfigFile::CVar &var)
	{
		while (ui.driverGraphComboBox->count())
			ui.driverGraphComboBox->removeItem(0);

		// load types graphics driver from the config file
		for (uint i = 0; i < var.size(); ++i)
			ui.driverGraphComboBox->addItem(var.asString(i).c_str());

		// set graphics driver from the config file
		QString value = Modules::config().getValue("GraphicsDriver",std::string("OpenGL")).c_str();
		QString dn = value.toLower();
		for (sint i = 0; i < ui.driverGraphComboBox->count(); ++i)
		{
			if (dn == ui.driverGraphComboBox->itemText(i).toLower())
			{
				ui.driverGraphComboBox->setCurrentIndex(i);
				return;
			}
		}
	}

	void CSettingsDialog::cfcbSearchPaths(NLMISC::CConfigFile::CVar &var)
	{
		ui.pathsListWidget->clear();

		// load search paths from the config file
		for (uint i = 0; i < var.size(); ++i)
		{
			ui.pathsListWidget->addItem(var.asString(i).c_str());
			ui.pathsListWidget->item(i)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		}
	}

	void CSettingsDialog::cfcbLeveldesignPath(NLMISC::CConfigFile::CVar &var)
	{
		// load leveldesign path from the config file	
		ui.leveldesignPath->setText(var.asString().c_str());
	}

	void CSettingsDialog::browseLeveldesignPath()
	{
		ui.leveldesignPath->setText(QFileDialog::getExistingDirectory(this, tr("Open Directory"),
			QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
	}

} /* namespace NLQT */
