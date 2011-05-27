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
#include "object_viewer_constants.h"
#include "../core/icore.h"
#include "modules.h"

// Qt includes
#include <QtGui/QWidget>
#include <QtCore/QSettings>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

namespace NLQT
{

QString LastDir = ".";

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
	return QLatin1String(Constants::OBJECT_VIEWER_SECTION);
}

QString VegetableSettingsPage::trCategory() const
{
	return tr("Object Viewer");
}

QIcon VegetableSettingsPage::categoryIcon() const
{
	return QIcon();
}

QWidget *VegetableSettingsPage::createPage(QWidget *parent)
{
	m_page = new QWidget(parent);
	m_ui.setupUi(m_page);

	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::OBJECT_VIEWER_SECTION);

	m_ui.tileBankLineEdit->setText(settings->value(Constants::VEGET_TILE_BANK, "").toString());
	m_ui.tileFarBankLineEdit->setText(settings->value(Constants::VEGET_TILE_FAR_BANK, "").toString());
	m_ui.vegetTextureLineEdit->setText(settings->value(Constants::VEGET_TEXTURE, "").toString());
	m_ui.coarseLineEdit->setText(settings->value(Constants::COARSE_MESH_TEXTURE, "").toString());
	m_ui.zonesListWidget->addItems(settings->value(Constants::VEGET_LANDSCAPE_ZONES).toStringList());

	settings->endGroup();

	connect(m_ui.tileBankToolButton, SIGNAL(clicked()), this, SLOT(setTileBank()));
	connect(m_ui.tileFarBankToolButton, SIGNAL(clicked()), this, SLOT(setTileFarBank()));
	connect(m_ui.vegetTexToolButton, SIGNAL(clicked()), this, SLOT(setTextureVegetable()));
	connect(m_ui.coarseToolButton, SIGNAL(clicked()), this, SLOT(setCoarseMeshTexture()));
	connect(m_ui.addZoneToolButton, SIGNAL(clicked()), this, SLOT(addZone()));
	connect(m_ui.removeZoneToolButton, SIGNAL(clicked()), this, SLOT(removeZone()));
	connect(m_ui.clearButton, SIGNAL(clicked()), m_ui.zonesListWidget, SLOT(clear()));

	return m_page;
}

void VegetableSettingsPage::apply()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::OBJECT_VIEWER_SECTION);

	settings->setValue(Constants::VEGET_TILE_BANK, m_ui.tileBankLineEdit->text());
	settings->setValue(Constants::VEGET_TILE_FAR_BANK, m_ui.tileFarBankLineEdit->text());
	settings->setValue(Constants::COARSE_MESH_TEXTURE, m_ui.coarseLineEdit->text());
	settings->setValue(Constants::VEGET_TEXTURE, m_ui.vegetTextureLineEdit->text());

	QStringList list;
	for (sint i = 0; i < m_ui.zonesListWidget->count(); ++i)
		list.push_back(m_ui.zonesListWidget->item(i)->text());

	settings->setValue(Constants::VEGET_LANDSCAPE_ZONES, list);

	settings->endGroup();
	settings->sync();
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

void VegetableSettingsPage::setCoarseMeshTexture()
{
	QString fileName = QFileDialog::getOpenFileName(0, tr("Set Coarse Mesh texture"),
					   m_ui.vegetTextureLineEdit->text(),
					   tr("Texture file (*.tga *.png *.jpg *.dds);;"));
	if (!fileName.isEmpty())
	{
		m_ui.coarseLineEdit->setText(fileName);
	}
}

void VegetableSettingsPage::addZone()
{
	QStringList fileNames = QFileDialog::getOpenFileNames(0,
							tr("Add zone files"), LastDir,
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