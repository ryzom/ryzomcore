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
#include "wizarddialog.h"
#include "configfile.h"

#include "nel/misc/system_info.h"
#include "nel/misc/common.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

QString qBytesToHumanReadable(qint64 bytes)
{
	static std::vector<std::string> units;

	if (units.empty())
	{
		units.push_back(QObject::tr("B").toUtf8().constData());
		units.push_back(QObject::tr("KiB").toUtf8().constData());
		units.push_back(QObject::tr("MiB").toUtf8().constData());
		units.push_back(QObject::tr("GiB").toUtf8().constData());
		units.push_back(QObject::tr("TiB").toUtf8().constData());
		units.push_back(QObject::tr("PiB").toUtf8().constData());
	}

	return QString::fromUtf8(NLMISC::bytesToHumanReadable(bytes).c_str());
}

CWizardDialog::CWizardDialog():QDialog()
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	setupUi(this);

	currentDirectoryRadioButton->setVisible(false);
	oldDirectoryRadioButton->setVisible(false);

	// enable download radio button by default
	internetRadioButton->setChecked(true);

	// if launched from current directory, it means we just patched files
	m_currentDirectory = CConfigFile::getInstance()->getCurrentDirectory();

	if (!CConfigFile::getInstance()->isRyzomInstalledIn(m_currentDirectory))
	{
		// current directory is a child of Ryzom root directory
		m_currentDirectory = CConfigFile::getInstance()->getParentDirectory();

		if (!CConfigFile::getInstance()->isRyzomInstalledIn(m_currentDirectory))
		{
			// Ryzom is in the same directory as Ryzom Installer
			m_currentDirectory = CConfigFile::getInstance()->getApplicationDirectory();

			if (!CConfigFile::getInstance()->isRyzomInstalledIn(m_currentDirectory))
			{
				m_currentDirectory.clear();
			}
		}
	}

	// display found directory
	if (!m_currentDirectory.isEmpty())
	{
		currentDirectoryRadioButton->setText(tr("Current directory: %1").arg(m_currentDirectory));
		currentDirectoryRadioButton->setVisible(true);
		currentDirectoryRadioButton->setChecked(true);
	}

	m_oldDirectory = CConfigFile::getInstance()->getOldInstallationDirectory();

	// found a previous installation
	if (CConfigFile::getInstance()->areRyzomDataInstalledIn(m_oldDirectory))
	{
		oldDirectoryRadioButton->setText(tr("Old installation: %1").arg(m_oldDirectory));
		oldDirectoryRadioButton->setVisible(true);

		if (m_currentDirectory.isEmpty()) oldDirectoryRadioButton->setChecked(true);
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

CWizardDialog::~CWizardDialog()
{
}

void CWizardDialog::onShowAdvancedParameters(int state)
{
	advancedFrame->setVisible(state != Qt::Unchecked);

	adjustSize();
}

void CWizardDialog::onAnotherLocationBrowseButtonClicked()
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

void CWizardDialog::onDestinationBrowseButtonClicked()
{
	QString directory = QFileDialog::getExistingDirectory(this, tr("Please choose directory where to install Ryzom"));

	if (directory.isEmpty()) return;

	m_dstDirectory = directory;

	updateDestinationText();
}

void CWizardDialog::updateAnotherLocationText()
{
	anotherLocationRadioButton->setText(tr("Another location: %1").arg(m_anotherDirectory.isEmpty() ? tr("Undefined"):m_anotherDirectory));
}

void CWizardDialog::updateDestinationText()
{
	destinationLabel->setText(m_dstDirectory);
}

void CWizardDialog::accept()
{
	// check free disk space
	qint64 freeSpace = NLMISC::CSystemInfo::availableHDSpace(m_dstDirectory.toUtf8().constData());

	const CServer &server = CConfigFile::getInstance()->getServer();

	if (freeSpace < server.dataUncompressedSize)
	{
	    QMessageBox::StandardButton res = QMessageBox::warning(this, tr("Not enough free disk space"), tr("You don't have enough free space on this disk, please make more space or choose a directory on another disk."));
		return;
	}

	if (currentDirectoryRadioButton->isChecked())
	{
		CConfigFile::getInstance()->setSrcServerDirectory(m_currentDirectory);
	}
	else if (oldDirectoryRadioButton->isChecked())
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
