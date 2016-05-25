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
#include "installwizarddialog.h"
#include "configfile.h"
#include "utils.h"

#include "nel/misc/system_info.h"
#include "nel/misc/common.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

CInstallWizardDialog::CInstallWizardDialog():QDialog()
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	setupUi(this);

	oldDirectoryRadioButton->setVisible(false);

	// enable download radio button by default
	internetRadioButton->setChecked(true);

	m_oldDirectory = CConfigFile::getInstance()->getOldInstallationDirectory();

	// found a previous installation
	if (CConfigFile::getInstance()->areRyzomDataInstalledIn(m_oldDirectory))
	{
		oldDirectoryRadioButton->setText(tr("Old installation: %1").arg(m_oldDirectory));
		oldDirectoryRadioButton->setVisible(true);
		oldDirectoryRadioButton->setChecked(true);
	}

	updateAnotherLocationText();

	m_dstDirectory = CConfigFile::getNewInstallationDirectory();

	updateDestinationText();

	// check whether OS architecture is 32 or 64 bits
	// TODO: 64 bits client only supported under Vista+
	if (CConfigFile::has64bitsOS())
	{
		clientArchGroupBox->setVisible(true);
		clientArch64RadioButton->setChecked(true);
	}
	else
	{
		clientArchGroupBox->setVisible(false);
		clientArch32RadioButton->setChecked(true);
	}

	const CServer &server = CConfigFile::getInstance()->getServer();

	internetRadioButton->setText(tr("Internet (%1 to download)").arg(qBytesToHumanReadable(server.dataCompressedSize)));
	destinationGroupBox->setTitle(tr("Files will be installed to (requires %1):").arg(qBytesToHumanReadable(server.dataUncompressedSize)));

	connect(anotherLocationBrowseButton, SIGNAL(clicked()), SLOT(onAnotherLocationBrowseButtonClicked()));
	connect(destinationBrowseButton, SIGNAL(clicked()), SLOT(onDestinationBrowseButtonClicked()));

	// TODO: if found a folder with initial data, get its total size and check if at least that size is free on the disk

	// by default, advanced parameters are hidden
	onShowAdvancedParameters(Qt::Unchecked);

	connect(advancedCheckBox, SIGNAL(stateChanged(int)), SLOT(onShowAdvancedParameters(int)));
}

CInstallWizardDialog::~CInstallWizardDialog()
{
}

void CInstallWizardDialog::onShowAdvancedParameters(int state)
{
	advancedFrame->setVisible(state != Qt::Unchecked);

	adjustSize();
}

void CInstallWizardDialog::onAnotherLocationBrowseButtonClicked()
{
	QString directory;
	
	for(;;)
	{
		directory = QFileDialog::getExistingDirectory(this, tr("Please choose directory where is installed Ryzom"));

		if (directory.isEmpty()) return;

		if (CConfigFile::getInstance()->isRyzomInstalledIn(directory)) break;

	    QMessageBox::StandardButton res = QMessageBox::warning(this, tr("Unable to find Ryzom"), tr("Unable to find Ryzom in selected directory. Please choose another one or cancel."));
	}

	m_anotherDirectory = directory;

	// if we browse to a Ryzom installation, we want to use it
	anotherLocationRadioButton->setChecked(true);

	updateAnotherLocationText();
}

void CInstallWizardDialog::onDestinationBrowseButtonClicked()
{
	QString directory = QFileDialog::getExistingDirectory(this, tr("Please choose directory where to install Ryzom"));

	if (directory.isEmpty()) return;

	m_dstDirectory = directory;

	updateDestinationText();
}

void CInstallWizardDialog::updateAnotherLocationText()
{
	anotherLocationRadioButton->setText(tr("Another location: %1").arg(m_anotherDirectory.isEmpty() ? tr("Undefined"):m_anotherDirectory));
}

void CInstallWizardDialog::updateDestinationText()
{
	destinationLabel->setText(m_dstDirectory);
}

void CInstallWizardDialog::accept()
{
	// check free disk space
	qint64 freeSpace = NLMISC::CSystemInfo::availableHDSpace(m_dstDirectory.toUtf8().constData());

	const CServer &server = CConfigFile::getInstance()->getServer();

	if (freeSpace < server.dataUncompressedSize)
	{
	    QMessageBox::StandardButton res = QMessageBox::warning(this, tr("Not enough free disk space"), tr("You don't have enough free space on this disk, please make more space or choose a directory on another disk."));
		return;
	}

	if (oldDirectoryRadioButton->isChecked())
	{
		CConfigFile::getInstance()->setSrcServerDirectory(m_oldDirectory);
	}
	else if (anotherLocationRadioButton->isChecked())
	{
		CConfigFile::getInstance()->setSrcServerDirectory(m_anotherDirectory);
	}
	else
	{
		CConfigFile::getInstance()->setSrcServerDirectory("");
	}

	CConfigFile::getInstance()->setInstallationDirectory(m_dstDirectory);
	CConfigFile::getInstance()->setUse64BitsClient(clientArch64RadioButton->isChecked());
	CConfigFile::getInstance()->save();

	QDialog::accept();
}
