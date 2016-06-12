// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "stdpch.h"
#include "settingsdialog.h"
#include "configfile.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

CSettingsDialog::CSettingsDialog(QWidget *parent):QDialog(parent)
{
	setupUi(this);

	CConfigFile *config = CConfigFile::getInstance();

	// only 64 bits OS can switch between 32 and 64 bits
	use64bitsClientsCheckBox->setEnabled(config->has64bitsOS());

	// read value from config
	use64bitsClientsCheckBox->setChecked(config->use64BitsClient());

	connect(installationDirectoryButton, SIGNAL(clicked()), SLOT(onInstallationDirectoryButtonClicked()));

	// resize layout depending on content and constraints
	adjustSize();

	// fix height because to left bitmap
	setFixedHeight(height());
}

CSettingsDialog::~CSettingsDialog()
{
}

void CSettingsDialog::accept()
{
	// TODO: add save code

	QDialog::accept();
}

void CSettingsDialog::onInstallationDirectoryButtonClicked()
{
	QString directory = QFileDialog::getExistingDirectory(this, tr("Please choose directory where to install Ryzom"));

	if (directory.isEmpty()) return;

//	m_dstDirectory = directory;

//	updateDestinationText();
}
