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
#include "migratedialog.h"
#include "configfile.h"
#include "utils.h"

#include "nel/misc/system_info.h"
#include "nel/misc/common.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

CMigrateDialog::CMigrateDialog():QDialog()
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	setupUi(this);

	// if launched from current directory, it means we just patched files
	m_currentDirectory = CConfigFile::getInstance()->getCurrentDirectory();

	if (!CConfigFile::getInstance()->isRyzomInstalledIn(m_currentDirectory))
	{
		// Ryzom is in the same directory as Ryzom Installer
		m_currentDirectory = CConfigFile::getInstance()->getInstallerCurrentDirPath();

		if (!CConfigFile::getInstance()->isRyzomInstalledIn(m_currentDirectory))
		{
			m_currentDirectory.clear();
		}
	}

	// update default destination
	onDestinationDefaultButtonClicked();

	// both 32 and 64 bits are working under Windows 64 bits

	// check whether OS architecture is 32 or 64 bits
	if (CConfigFile::has64bitsOS())
	{
		// 64 bits enbabled by default
		clientArchGroupBox->setVisible(true);
		clientArch64RadioButton->setChecked(true);
		clientArch32RadioButton->setChecked(false);
	}
	else
	{
		// only 32 bits is available
		clientArchGroupBox->setVisible(false);
		clientArch64RadioButton->setChecked(false);
		clientArch32RadioButton->setChecked(true);
	}

	const CServer &server = CConfigFile::getInstance()->getServer();

	destinationGroupBox->setTitle(tr("Files will be installed to (requires %1):").arg(qBytesToHumanReadable(server.dataUncompressedSize)));

	connect(destinationDefaultButton, SIGNAL(clicked()), SLOT(onDestinationDefaultButtonClicked()));
	connect(destinationBrowseButton, SIGNAL(clicked()), SLOT(onDestinationBrowseButtonClicked()));
	connect(continueButton, SIGNAL(clicked()), SLOT(accept()));
	connect(quitButton, SIGNAL(clicked()), SLOT(reject()));

	// TODO: if found a folder with initial data, get its total size and check if at least that size is free on the disk

	// by default, advanced parameters are hidden
	onShowAdvancedParameters(Qt::Unchecked);

	connect(advancedCheckBox, SIGNAL(stateChanged(int)), SLOT(onShowAdvancedParameters(int)));

	raise();
}

CMigrateDialog::~CMigrateDialog()
{
}

void CMigrateDialog::onShowAdvancedParameters(int state)
{
	advancedFrame->setVisible(state != Qt::Unchecked);

	adjustSize();
}

void CMigrateDialog::onDestinationDefaultButtonClicked()
{
	m_dstDirectory = CConfigFile::getNewInstallationDirectory();

	updateDestinationText();
}

void CMigrateDialog::onDestinationBrowseButtonClicked()
{
	QString directory = QFileDialog::getExistingDirectory(this, tr("Please choose directory to install Ryzom in"), m_dstDirectory);

	if (directory.isEmpty()) return;

	m_dstDirectory = directory;

	updateDestinationText();
}

void CMigrateDialog::updateDestinationText()
{
	destinationLabel->setText(m_dstDirectory);
}

void CMigrateDialog::accept()
{
	// check free disk space
	bool ignoreFreeDiskSpaceChecks = CConfigFile::getInstance()->ignoreFreeDiskSpaceChecks();
	qint64 freeSpace = NLMISC::CSystemInfo::availableHDSpace(m_dstDirectory.toUtf8().constData());

	// shouldn't happen
	if (!ignoreFreeDiskSpaceChecks && freeSpace == 0)
	{
		int error = NLMISC::getLastError();

		nlwarning("Error '%s' (%d) occurred when trying to check free disk space on %s, continue anyway", NLMISC::formatErrorMessage(error).c_str(), error, Q2C(m_dstDirectory));
	}

	// compare with exact size of current directory
	if (!ignoreFreeDiskSpaceChecks && freeSpace && freeSpace < getDirectorySize(m_currentDirectory, true))
	{
		QMessageBox::StandardButton res = QMessageBox::warning(this, tr("Not enough free disk space"), tr("You don't have enough free space on this disk, please make more space or choose a directory on another disk."));
		return;
	}

	// create directory if doesn't exist
	bool succeedsToWrite = QDir().mkpath(m_dstDirectory);

	// if unable to create directory, don't expect to write a file in it
	if (succeedsToWrite)
	{
		// check if directory is writable by current user
		if (!isDirectoryWritable(m_dstDirectory))
		{
			succeedsToWrite = false;
		}
	}

	if (!succeedsToWrite)
	{
		QMessageBox::StandardButton res = QMessageBox::warning(this, tr("Unable to write in directory"), tr("You don't have the permission to write in this directory with your current user account, please choose another directory."));
		return;
	}

	// if reinstalling in same directory, don't check if directory is empty
	if (m_dstDirectory != CConfigFile::getInstance()->getNewInstallationDirectory())
	{
		if (!isDirectoryEmpty(m_dstDirectory, true))
		{
			QMessageBox::StandardButton res = QMessageBox::warning(this, tr("Directory not empty"), tr("This directory is not empty, please choose another one."));
			return;
		}
	}

	CConfigFile::getInstance()->setSrcServerDirectory(m_currentDirectory);
	CConfigFile::getInstance()->setInstallationDirectory(m_dstDirectory);
	CConfigFile::getInstance()->setUse64BitsClient(clientArch64RadioButton->isChecked());
	CConfigFile::getInstance()->save();

	QDialog::accept();
}
