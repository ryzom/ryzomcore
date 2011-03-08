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
#include "stdpch.h"
#include "vegetable_settings_page.h"
#include "modules.h"

// Qt includes
#include <QtGui/QWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

// NeL includes
#include <nel/misc/config_file.h>

namespace NLQT
{

VegetableSettingsPage::VegetableSettingsPage(QObject *parent)
	: IOptionsPage(parent),
	  m_page(0)
{
}

QString VegetableSettingsPage::id() const
{
	return QLatin1String("VegetablePage");
}

QString VegetableSettingsPage::trName() const
{
	return tr("Vegetable");
}

QString VegetableSettingsPage::category() const
{
	return QLatin1String("ObjectViewer");
}

QString VegetableSettingsPage::trCategory() const
{
	return tr("Object Viewer");
}

QWidget *VegetableSettingsPage::createPage(QWidget *parent)
{
	m_page = new QWidget(parent);
	m_ui.setupUi(m_page);

	m_ui.tileBankLineEdit->setText(Modules::config().getConfigFile().getVar("VegetTileBank").asString().c_str());
	m_ui.tileFarBankLineEdit->setText(Modules::config().getConfigFile().getVar("VegetTileFarBank").asString().c_str());
	m_ui.vegetTextureLineEdit->setText(Modules::config().getConfigFile().getVar("VegetTexture").asString().c_str());

	m_ui.zonesListWidget->clear();

	// load vegetable landscape zone paths from config file
	NLMISC::CConfigFile::CVar &var = Modules::config().getConfigFile().getVar("VegetLandscapeZones");
	for (uint i = 0; i < var.size(); ++i)
	{
		m_ui.zonesListWidget->addItem(var.asString(i).c_str());
		m_ui.zonesListWidget->item(i)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	}

	connect(m_ui.tileBankToolButton, SIGNAL(clicked()), this, SLOT(setTileBank()));
	connect(m_ui.tileFarBankToolButton, SIGNAL(clicked()), this, SLOT(setTileFarBank()));
	connect(m_ui.vegetTexToolButton, SIGNAL(clicked()), this, SLOT(setTextureVegetable()));
	connect(m_ui.addZoneToolButton, SIGNAL(clicked()), this, SLOT(addZone()));
	connect(m_ui.removeZoneToolButton, SIGNAL(clicked()), this, SLOT(removeZone()));

	return m_page;
}

void VegetableSettingsPage::apply()
{
	Modules::config().getConfigFile().getVar("VegetTileBank").setAsString(m_ui.tileBankLineEdit->text().toStdString());
	Modules::config().getConfigFile().getVar("VegetTileFarBank").setAsString(m_ui.tileFarBankLineEdit->text().toStdString());
	Modules::config().getConfigFile().getVar("VegetTexture").setAsString(m_ui.vegetTextureLineEdit->text().toStdString());

	std::vector<std::string> list;
	for (sint i = 0; i < m_ui.zonesListWidget->count(); ++i)
	{
		std::string str = m_ui.zonesListWidget->item(i)->text().toStdString();
		list.push_back(str);
	}

	Modules::config().getConfigFile().getVar("VegetLandscapeZones").Type = NLMISC::CConfigFile::CVar::T_STRING;
	Modules::config().getConfigFile().getVar("VegetLandscapeZones").setAsString(list);

	Modules::config().getConfigFile().save();
}

void VegetableSettingsPage::finish()
{
}

void VegetableSettingsPage::setTileBank()
{
	QString fileName = QFileDialog::getOpenFileName(0, tr("Set new tile bank"),
					   m_ui.tileBankLineEdit->text(),
					   tr("Tile Bank file (*.smallbank *.bank);;"));
	if (!fileName.isEmpty())
	{
		m_ui.tileBankLineEdit->setText(fileName);
	}
}

void VegetableSettingsPage::setTileFarBank()
{
	QString fileName = QFileDialog::getOpenFileName(0, tr("Set new tile far bank"),
					   m_ui.tileFarBankLineEdit->text(),
					   tr("Tile Far Bank file (*.farbank);;"));
	if (!fileName.isEmpty())
	{
		m_ui.tileFarBankLineEdit->setText(fileName);
	}
}

void VegetableSettingsPage::setTextureVegetable()
{
	QString fileName = QFileDialog::getOpenFileName(0, tr("Set MicroVegetable texture"),
					   m_ui.vegetTextureLineEdit->text(),
					   tr("Texture file (*.tga *.png *.jpg *.dds);;"));
	if (!fileName.isEmpty())
	{
		m_ui.vegetTextureLineEdit->setText(fileName);
	}
}

void VegetableSettingsPage::addZone()
{
	QStringList fileNames = QFileDialog::getOpenFileNames(0,
							tr("Add zone files"), ".",
							tr("Zonel files (*.zonel *.zone);;"));

	if (!fileNames.isEmpty())
	{
		QStringList list = fileNames;
		QStringList::Iterator it = list.begin();
		while(it != list.end())
		{
			QListWidgetItem *newItem = new QListWidgetItem;
			newItem->setText(*it);
			newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui.zonesListWidget->addItem(newItem);
			++it;
		}
	}
}

void VegetableSettingsPage::removeZone()
{
	QListWidgetItem *removeItem = m_ui.zonesListWidget->takeItem(m_ui.zonesListWidget->currentRow());
	if (!removeItem)
		delete removeItem;
}

} /* namespace NLQT */