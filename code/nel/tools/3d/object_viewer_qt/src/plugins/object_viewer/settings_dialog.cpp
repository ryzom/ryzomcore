/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

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

#include "stdpch.h"
#include "settings_dialog.h"

// Qt includes
#include <QtGui/QWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QStyleFactory>
#include <QtGui/QStyle>

// NeL includes
#include <nel/misc/path.h>
#include <nel/3d/bloom_effect.h>

// Project includes
#include "modules.h"

using namespace NLMISC;

namespace NLQT
{

CSettingsDialog::CSettingsDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	loadGraphicsSettings();
	loadSoundSettings();
	loadVegetableSettings();

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(applyPressed()));

	connect(ui.enableBloomCheckBox, SIGNAL(toggled(bool)), this, SLOT(setEnableBloom(bool)));
	connect(ui.squareBloomCheckBox, SIGNAL(toggled(bool)), this, SLOT(setEnableSquareBloon(bool)));
	connect(ui.bloomDensityHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setDensityBloom(int)));

	connect(ui.tileBankToolButton, SIGNAL(clicked()), this, SLOT(setTileBank()));
	connect(ui.tileFarBankToolButton, SIGNAL(clicked()), this, SLOT(setTileFarBank()));
	connect(ui.vegetTexToolButton, SIGNAL(clicked()), this, SLOT(setTextureVegetable()));
	connect(ui.addZoneToolButton, SIGNAL(clicked()), this, SLOT(addZone()));
	connect(ui.removeZoneToolButton, SIGNAL(clicked()), this, SLOT(removeZone()));

#ifdef NL_OS_UNIX
	ui.driverGraphComboBox->setEnabled(false);
#endif

}

CSettingsDialog::~CSettingsDialog()
{
	Modules::config().dropCallback("GraphicsDrivers");
	Modules::config().dropCallback("SoundDrivers");
}

void CSettingsDialog::applyPressed()
{

	// settings take after restart the program
	QMessageBox::warning(this, tr("Settings"),
						 tr("Graphics and sound settings "
							"take after restart the program"),
						 QMessageBox::Ok);

	saveGraphicsSettings();
	saveSoundSettings();
	saveVegetableSettings();

	// save config file
	Modules::config().getConfigFile().save();
}

void CSettingsDialog::setTileBank()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Set new tile bank"),
					   ui.tileBankLineEdit->text(),
					   tr("Tile Bank file (*.smallbank *.bank);;"));
	if (!fileName.isEmpty())
	{
		ui.tileBankLineEdit->setText(fileName);
	}
}

void CSettingsDialog::setTileFarBank()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Set new tile far bank"),
					   ui.tileFarBankLineEdit->text(),
					   tr("Tile Far Bank file (*.farbank);;"));
	if (!fileName.isEmpty())
	{
		ui.tileFarBankLineEdit->setText(fileName);
	}
}

void CSettingsDialog::setTextureVegetable()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Set MicroVegetable texture"),
					   ui.vegetTextureLineEdit->text(),
					   tr("Texture file (*.tga *.png *.jpg *.dds);;"));
	if (!fileName.isEmpty())
	{
		ui.vegetTextureLineEdit->setText(fileName);
	}
}

void CSettingsDialog::addZone()
{
	QStringList fileNames = QFileDialog::getOpenFileNames(this,
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
			ui.zonesListWidget->addItem(newItem);
			++it;
		}
	}
}

void CSettingsDialog::removeZone()
{
	QListWidgetItem *removeItem = ui.zonesListWidget->takeItem(ui.zonesListWidget->currentRow());
	if (!removeItem) delete removeItem;
}

void CSettingsDialog::setEnableBloom(bool state)
{
	Modules::objView().setBloomEffect(state);
}

void CSettingsDialog::setEnableSquareBloon(bool state)
{
	NL3D::CBloomEffect::instance().setSquareBloom(state);
}

void CSettingsDialog::setDensityBloom(int density)
{
	NL3D::CBloomEffect::instance().setDensityBloom(density);
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

void CSettingsDialog::cfcbSoundDrivers(NLMISC::CConfigFile::CVar& var)
{
	while (ui.driverSndComboBox->count())
		ui.driverSndComboBox->removeItem(0);

	// load types sound driver from the config file
	for (uint i = 0; i < var.size(); ++i)
		ui.driverSndComboBox->addItem(var.asString(i).c_str());

	// set sound driver from the config file
	QString value = Modules::config().getValue("SoundDriver",std::string("Auto")).c_str();
	QString dn = value.toLower();
	for (sint i = 0; i < ui.driverSndComboBox->count(); ++i)
	{
		if (dn == ui.driverSndComboBox->itemText(i).toLower())
		{
			ui.driverSndComboBox->setCurrentIndex(i);
			return;
		}
	}
}

void CSettingsDialog::loadGraphicsSettings()
{
	// setup config file callbacks and initialize values
	Modules::config().setAndCallback("GraphicsDrivers", CConfigCallback(this, &CSettingsDialog::cfcbGraphicsDrivers));

	ui.enableBloomCheckBox->setChecked(Modules::objView().getBloomEffect());
	ui.squareBloomCheckBox->setChecked(NL3D::CBloomEffect::instance().getSquareBloom());
	ui.bloomDensityHorizontalSlider->setValue(NL3D::CBloomEffect::instance().getDensityBloom());

	ui.styleComboBox->addItems(QStyleFactory::keys());

	ui.styleComboBox->setCurrentIndex(ui.styleComboBox->findText(Modules::config().getValue("QtStyle", std::string("")).c_str()));

	ui.paletteCheckBox->setChecked(Modules::config().getValue("QtPalette", false));
}

void CSettingsDialog::loadSoundSettings()
{
	// setup config file callbacks and initialize values
	Modules::config().setAndCallback("SoundDrivers", CConfigCallback(this, &CSettingsDialog::cfcbSoundDrivers));

	// load settings from the config file
	ui.autoLoadSampleCheckBox->setChecked(Modules::config().getValue("SoundAutoLoadSample", true));
	ui.enableOccludeObstructCheckBox->setChecked(Modules::config().getValue("SoundEnableOccludeObstruct", true));
	ui.enableReverbCheckBox->setChecked(Modules::config().getValue("SoundEnableReverb", true));
	ui.manualRolloffCheckBox->setChecked(Modules::config().getValue("SoundManualRolloff", true));
	ui.forceSoftwareCheckBox->setChecked(Modules::config().getValue("SoundForceSoftware", false));
	ui.useADPCMCheckBox->setChecked(Modules::config().getValue("SoundUseADPCM", false));
	ui.maxTrackSpinBox->setValue(Modules::config().getValue("SoundMaxTrack", 48));
}

void CSettingsDialog::loadVegetableSettings()
{
	ui.tileBankLineEdit->setText(Modules::config().getConfigFile().getVar("VegetTileBank").asString().c_str());
	ui.tileFarBankLineEdit->setText(Modules::config().getConfigFile().getVar("VegetTileFarBank").asString().c_str());
	ui.vegetTextureLineEdit->setText(Modules::config().getConfigFile().getVar("VegetTexture").asString().c_str());

	ui.zonesListWidget->clear();

	// load vegetable landscape zone paths from config file
	NLMISC::CConfigFile::CVar &var = Modules::config().getConfigFile().getVar("VegetLandscapeZones");
	for (uint i = 0; i < var.size(); ++i)
	{
		ui.zonesListWidget->addItem(var.asString(i).c_str());
		ui.zonesListWidget->item(i)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	}
}

void CSettingsDialog::saveGraphicsSettings()
{
	// save graphics settings to config file
	Modules::config().getConfigFile().getVar("GraphicsDriver").setAsString(ui.driverGraphComboBox->currentText().toStdString());

	Modules::config().getConfigFile().getVar("BloomEffect").setAsInt(ui.enableBloomCheckBox->isChecked());
	Modules::config().getConfigFile().getVar("BloomSquare").setAsInt(ui.squareBloomCheckBox->isChecked());
	Modules::config().getConfigFile().getVar("BloomDensity").setAsInt(ui.bloomDensityHorizontalSlider->value());

	Modules::config().getConfigFile().getVar("QtStyle").setAsString(ui.styleComboBox->currentText().toStdString());
	Modules::config().getConfigFile().getVar("QtPalette").setAsInt(ui.paletteCheckBox->isChecked());

	QApplication::setStyle(QStyleFactory::create(ui.styleComboBox->currentText()));

	if (ui.paletteCheckBox->isChecked())
		QApplication::setPalette(QApplication::style()->standardPalette());
	else
		QApplication::setPalette(Modules::mainWin().getOriginalPalette());
}

void CSettingsDialog::saveSoundSettings()
{
	// save sound settings to config file
	Modules::config().getConfigFile().getVar("SoundDriver").setAsString(ui.driverSndComboBox->currentText().toStdString());
	Modules::config().getConfigFile().getVar("SoundAutoLoadSample").setAsInt(ui.autoLoadSampleCheckBox->isChecked());
	Modules::config().getConfigFile().getVar("SoundEnableOccludeObstruct").setAsInt(ui.enableOccludeObstructCheckBox->isChecked());
	Modules::config().getConfigFile().getVar("SoundEnableReverb").setAsInt(ui.enableReverbCheckBox->isChecked());
	Modules::config().getConfigFile().getVar("SoundManualRolloff").setAsInt(ui.manualRolloffCheckBox->isChecked());
	Modules::config().getConfigFile().getVar("SoundForceSoftware").setAsInt(ui.forceSoftwareCheckBox->isChecked());
	Modules::config().getConfigFile().getVar("SoundUseADPCM").setAsInt(ui.useADPCMCheckBox->isChecked());
	Modules::config().getConfigFile().getVar("SoundMaxTrack").setAsInt(ui.maxTrackSpinBox->value());
}

void CSettingsDialog::saveVegetableSettings()
{
	Modules::config().getConfigFile().getVar("VegetTileBank").setAsString(ui.tileBankLineEdit->text().toStdString());
	Modules::config().getConfigFile().getVar("VegetTileFarBank").setAsString(ui.tileFarBankLineEdit->text().toStdString());
	Modules::config().getConfigFile().getVar("VegetTexture").setAsString(ui.vegetTextureLineEdit->text().toStdString());

	std::vector<std::string> list;
	for (sint i = 0; i < ui.zonesListWidget->count(); ++i)
	{
		std::string str = ui.zonesListWidget->item(i)->text().toStdString();
		list.push_back(str);
	}

	Modules::config().getConfigFile().getVar("VegetLandscapeZones").Type = NLMISC::CConfigFile::CVar::T_STRING;
	Modules::config().getConfigFile().getVar("VegetLandscapeZones").setAsString(list);
}

} /* namespace NLQT */