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
#include "object_viewer_constants.h"
#include "../core/icore.h"
#include "modules.h"

// Qt includes
#include <QtCore/QSettings>
#include <QtGui/QWidget>
#include <QtGui/QFileDialog>

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
	return QLatin1String(Constants::OBJECT_VIEWER_SECTION);
}

QString SoundSettingsPage::trCategory() const
{
	return tr("Object Viewer");
}

QIcon SoundSettingsPage::categoryIcon() const
{
	return QIcon();
}

QWidget *SoundSettingsPage::createPage(QWidget *parent)
{
	m_page = new QWidget(parent);
	m_ui.setupUi(m_page);

	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::OBJECT_VIEWER_SECTION);

	QString soundDriver = settings->value(Constants::SOUND_DRIVER, "Auto").toString();
	m_ui.driverSndComboBox->setCurrentIndex(m_ui.driverSndComboBox->findText(soundDriver));

	// load settings from the config file
	m_ui.autoLoadSampleCheckBox->setChecked(settings->value(Constants::SOUND_AUTO_LOAD_SAMPLE, true).toBool());
	m_ui.enableOccludeObstructCheckBox->setChecked(settings->value(Constants::SOUND_ENABLE_OCCLUDE_OBSTRUCT, true).toBool());
	m_ui.enableReverbCheckBox->setChecked(settings->value(Constants::SOUND_ENABLE_REVERB, true).toBool());
	m_ui.manualRolloffCheckBox->setChecked(settings->value(Constants::SOUND_MANUAL_ROLL_OFF, true).toBool());
	m_ui.forceSoftwareCheckBox->setChecked(settings->value(Constants::SOUND_FORCE_SOFTWARE, false).toBool());
	m_ui.useADPCMCheckBox->setChecked(settings->value(Constants::SOUND_USE_ADCPM, false).toBool());
	m_ui.maxTrackSpinBox->setValue(settings->value(Constants::SOUND_MAX_TRACK, 48).toInt());
	m_ui.soundSamplePathLineEdit->setText(settings->value(Constants::SOUND_SAMPLE_PATH, "").toString());
	m_ui.soundSheetPathLineEdit->setText(settings->value(Constants::SOUND_PACKED_SHEET_PATH, "").toString());

	connect(m_ui.soundSamplePathButton, SIGNAL(clicked()), this, SLOT(setSamplePath()));
	connect(m_ui.soundSheetPathButton, SIGNAL(clicked()), this, SLOT(setSheetPath()));

	settings->endGroup();
	return m_page;
}

void SoundSettingsPage::apply()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::OBJECT_VIEWER_SECTION);

	settings->setValue(Constants::SOUND_DRIVER, m_ui.driverSndComboBox->currentText());
	settings->setValue(Constants::SOUND_AUTO_LOAD_SAMPLE, m_ui.autoLoadSampleCheckBox->isChecked());
	settings->setValue(Constants::SOUND_ENABLE_OCCLUDE_OBSTRUCT, m_ui.enableOccludeObstructCheckBox->isChecked());
	settings->setValue(Constants::SOUND_ENABLE_REVERB, m_ui.enableReverbCheckBox->isChecked());
	settings->setValue(Constants::SOUND_MANUAL_ROLL_OFF, m_ui.manualRolloffCheckBox->isChecked());
	settings->setValue(Constants::SOUND_FORCE_SOFTWARE, m_ui.forceSoftwareCheckBox->isChecked());
	settings->setValue(Constants::SOUND_USE_ADCPM, m_ui.useADPCMCheckBox->isChecked());
	settings->setValue(Constants::SOUND_MAX_TRACK, m_ui.maxTrackSpinBox->value());
	settings->setValue(Constants::SOUND_SAMPLE_PATH, m_ui.soundSamplePathLineEdit->text());
	settings->setValue(Constants::SOUND_PACKED_SHEET_PATH, m_ui.soundSheetPathLineEdit->text());

	settings->endGroup();
	settings->sync();
}

void SoundSettingsPage::finish()
{
}

void SoundSettingsPage::setSheetPath()
{
	QString path = QFileDialog::getExistingDirectory();
	if (!path.isEmpty())
	{
		m_ui.soundSheetPathLineEdit->setText(path);
	}
}

void SoundSettingsPage::setSamplePath()
{
	QString path = QFileDialog::getExistingDirectory();
	if (!path.isEmpty())
	{
		m_ui.soundSamplePathLineEdit->setText(path);
	}
}

} /* namespace NLQT */