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
#include "particle_sound_page.h"

// Qt includes
#include <QtGui/QInputDialog>

// NeL includes
#include "nel/3d/ps_located.h"
#include "nel/3d/u_particle_system_sound.h"
#include "nel/3d/particle_system.h"

// Project includes
#include "modules.h"
#include "sound_system.h"

namespace NLQT
{

CSoundPage::CSoundPage(QWidget *parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	// setup dialog the sounds gain
	_ui.gainWidget->setRange(0.f, 1.f);
	_ui.gainWidget->setWrapper(&_GainWrapper);
	_ui.gainWidget->setSchemeWrapper(&_GainWrapper);
	_ui.gainWidget->init();

	// setup dialog the sounds pitch
	_ui.pitchWidget->setRange(0.001f, 5.f);
	_ui.pitchWidget->setWrapper(&_PitchWrapper);
	_ui.pitchWidget->setSchemeWrapper(&_PitchWrapper);
	_ui.pitchWidget->init();

	// setup dialog the percent of sound emissions
	_ui.emissionWidget->setRange(0.f, 1.f);

	connect(_ui.browsePushButton ,SIGNAL(clicked()), this, SLOT(browse()));
	connect(_ui.playPushButton ,SIGNAL(clicked()), this, SLOT(play()));
	connect(_ui.spawnCheckBox ,SIGNAL(toggled(bool)), this, SLOT(setSpawn(bool)));
	connect(_ui.muteCheckBox ,SIGNAL(toggled(bool)), this, SLOT(setMute(bool)));
	connect(_ui.keepPitchCheckBox ,SIGNAL(toggled(bool)), this, SLOT(setKeepPitch(bool)));
	connect(_ui.soundNameLineEdit ,SIGNAL(textChanged(QString)), this, SLOT(setSoundName(QString)));

	connect(_ui.emissionWidget, SIGNAL(valueChanged(float)), this, SLOT(setEmissionPercent(float)));
}

CSoundPage::~CSoundPage()
{
}

void CSoundPage::setEditedItem(CWorkspaceNode *ownerNode, NL3D::CPSLocatedBindable *locatedBindable)
{
	_Sound = static_cast<NL3D::CPSSound *>(locatedBindable);
	_Node = ownerNode;

	nlassert(_Sound);

	_ui.emissionWidget->setValue(_Sound->getEmissionPercent(), false);

	_GainWrapper.S = _Sound;
	_ui.gainWidget->setWorkspaceNode(_Node);
	_ui.gainWidget->updateUi();

	_PitchWrapper.S = _Sound;
	_ui.pitchWidget->setWorkspaceNode(_Node);
	_ui.pitchWidget->updateUi();

	_ui.soundNameLineEdit->setText(QString(_Sound->getSoundName().toString().c_str()));

	_ui.spawnCheckBox->setChecked(_Sound->getSpawn());
	_ui.muteCheckBox->setChecked(_Sound->getMute());
	_ui.keepPitchCheckBox->setChecked(_Sound->getUseOriginalPitchFlag());
}

void CSoundPage::browse()
{
	std::vector<NLMISC::CSheetId> names;


	NLSOUND::UAudioMixer *audioMixer = Modules::sound().getAudioMixer();
	if (audioMixer)
	{
		audioMixer->getSoundNames(names);
	}

	// TODO: create CPickSound dialog
	QStringList items;
	items << tr("");
	for(size_t i = 0; i < names.size(); ++i)
		items << QString(names[i].toString().c_str());

	bool ok;
	QString item = QInputDialog::getItem(this, tr("Select your sound"),
										 tr("Sound:"), items, 0, false, &ok);
	if (ok)
	{
		_ui.soundNameLineEdit->setText(item);
		updateModifiedFlag();
	}
}

void CSoundPage::play()
{
	Modules::sound().play(_ui.soundNameLineEdit->text().toUtf8().constData());
}

void CSoundPage::setSpawn(bool state)
{
	if (state != _Sound->getSpawn())
	{
		_Sound->setSpawn(state);
		updateModifiedFlag();
	}
}

void CSoundPage::setMute(bool state)
{
	if (state != _Sound->getMute())
	{
		_Sound->setMute(state);
		updateModifiedFlag();
	}
}

void CSoundPage::setKeepPitch(bool state)
{
	bool hadScheme = _PitchWrapper.getScheme() != NULL;
	if (state != _Sound->getUseOriginalPitchFlag())
	{
		_Sound->setUseOriginalPitchFlag(state);
		updateModifiedFlag();
	}
	if (state)
	{
		if (hadScheme) _ui.pitchWidget->updateUi();
		///!!!!!
		///_PitchDlg->closeEditWindow();
	}
	_ui.pitchWidget->setEnabled(!state);
}

void CSoundPage::setSoundName(const QString &text)
{
	_Sound->setSoundName(NLMISC::CSheetId(text.toUtf8().constData()));
}

void CSoundPage::setEmissionPercent(float value)
{
	_Sound->setEmissionPercent(value);
	updateModifiedFlag();
}

} /* namespace NLQT */
