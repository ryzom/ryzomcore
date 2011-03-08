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
#include "sound_settings_page.h"
#include "modules.h"

// Qt includes
#include <QtGui/QWidget>


namespace NLQT
{

SoundSettingsPage::SoundSettingsPage(QObject *parent)
	: IOptionsPage(parent),
	  m_page(0)
{
}

QString SoundSettingsPage::id() const
{
	return QLatin1String("SoundPage");
}

QString SoundSettingsPage::trName() const
{
	return tr("Sound");
}

QString SoundSettingsPage::category() const
{
	return QLatin1String("ObjectViewer");
}

QString SoundSettingsPage::trCategory() const
{
	return tr("Object Viewer");
}

QWidget *SoundSettingsPage::createPage(QWidget *parent)
{
	m_page = new QWidget(parent);
	m_ui.setupUi(m_page);

	// setup config file callbacks and initialize values
	Modules::config().setAndCallback("SoundDrivers", CConfigCallback(this, &SoundSettingsPage::cfcbSoundDrivers));

	// load settings from the config file
	m_ui.autoLoadSampleCheckBox->setChecked(Modules::config().getValue("SoundAutoLoadSample", true));
	m_ui.enableOccludeObstructCheckBox->setChecked(Modules::config().getValue("SoundEnableOccludeObstruct", true));
	m_ui.enableReverbCheckBox->setChecked(Modules::config().getValue("SoundEnableReverb", true));
	m_ui.manualRolloffCheckBox->setChecked(Modules::config().getValue("SoundManualRolloff", true));
	m_ui.forceSoftwareCheckBox->setChecked(Modules::config().getValue("SoundForceSoftware", false));
	m_ui.useADPCMCheckBox->setChecked(Modules::config().getValue("SoundUseADPCM", false));
	m_ui.maxTrackSpinBox->setValue(Modules::config().getValue("SoundMaxTrack", 48));

	return m_page;
}

void SoundSettingsPage::apply()
{
	// save sound settings to config file
	Modules::config().getConfigFile().getVar("SoundDriver").setAsString(m_ui.driverSndComboBox->currentText().toStdString());
	Modules::config().getConfigFile().getVar("SoundAutoLoadSample").setAsInt(m_ui.autoLoadSampleCheckBox->isChecked());
	Modules::config().getConfigFile().getVar("SoundEnableOccludeObstruct").setAsInt(m_ui.enableOccludeObstructCheckBox->isChecked());
	Modules::config().getConfigFile().getVar("SoundEnableReverb").setAsInt(m_ui.enableReverbCheckBox->isChecked());
	Modules::config().getConfigFile().getVar("SoundManualRolloff").setAsInt(m_ui.manualRolloffCheckBox->isChecked());
	Modules::config().getConfigFile().getVar("SoundForceSoftware").setAsInt(m_ui.forceSoftwareCheckBox->isChecked());
	Modules::config().getConfigFile().getVar("SoundUseADPCM").setAsInt(m_ui.useADPCMCheckBox->isChecked());
	Modules::config().getConfigFile().getVar("SoundMaxTrack").setAsInt(m_ui.maxTrackSpinBox->value());

	// save config file
	Modules::config().getConfigFile().save();
}

void SoundSettingsPage::finish()
{
	Modules::config().dropCallback("SoundDrivers");
}

void SoundSettingsPage::cfcbSoundDrivers(NLMISC::CConfigFile::CVar& var)
{
	while (m_ui.driverSndComboBox->count())
		m_ui.driverSndComboBox->removeItem(0);

	// load types sound driver from the config file
	for (uint i = 0; i < var.size(); ++i)
		m_ui.driverSndComboBox->addItem(var.asString(i).c_str());

	// set sound driver from the config file
	QString value = Modules::config().getValue("SoundDriver",std::string("Auto")).c_str();
	QString dn = value.toLower();
	for (sint i = 0; i < m_ui.driverSndComboBox->count(); ++i)
	{
		if (dn == m_ui.driverSndComboBox->itemText(i).toLower())
		{
			m_ui.driverSndComboBox->setCurrentIndex(i);
			return;
		}
	}
}

} /* namespace NLQT */