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
		m_currentDirectory = CConfigFile::getInstance()->getApplicationDirectory();

		if (!CConfigFile::getInstance()->isRyzomInstalledIn(m_currentDirectory))
		{
			m_currentDirectory.clear();
		}
	}

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

	destinationGroupBox->setTitle(tr("Files will be installed to (requires %1):").arg(qBytesToHumanReadable(server.dataUncompressedSize)));

	connect(destinationBrowseButton, SIGNAL(clicked()), SLOT(onDestinationBrowseButtonClicked()));
	connect(continueButton, SIGNAL(clicked()), SLOT(accept()));
	connect(quitButton, SIGNAL(clicked()), SLOT(reject()));

	// TODO: if found a folder with initial data, get its total size and check if at least that size is free on the disk

	// by default, advanced parameters are hidden
	onShowAdvancedParameters(Qt::Unchecked);

	connect(advancedCheckBox, SIGNAL(stateChanged(int)), SLOT(onShowAdvancedParameters(int)));
}

CMigrateDialog::~CMigrateDialog()
{
}

void CMigrateDialog::onShowAdvancedParameters(int state)
{
	advancedFrame->setVisible(state != Qt::Unchecked);

	adjustSize();
}

void CMigrateDialog::onDestinationBrowseButtonClicked()
{
	QString directory = QFileDialog::getExistingDirectory(this, tr("Please choose directory where to install Ryzom"));

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
	qint64 freeSpace = NLMISC::CSystemInfo::availableHDSpace(m_dstDirectory.toUtf8().constData());

	const CServer &server = CConfigFile::getInstance()->getServer();

	if (freeSpace < server.dataUncompressedSize)
	{
	    QMessageBox::StandardButton res = QMessageBox::warning(this, tr("Not enough free disk space"), tr("You don't have enough free space on this disk, please make more space or choose a directory on another disk."));
		return;
	}

	CConfigFile::getInstance()->setSrcServerDirectory(m_currentDirectory);
	CConfigFile::getInstance()->setInstallationDirectory(m_dstDirectory);
	CConfigFile::getInstance()->setUse64BitsClient(clientArch64RadioButton->isChecked());
	CConfigFile::getInstance()->save();

	QDialog::accept();
}
