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
#include "profilesdialog.h"
#include "profilesmodel.h"
#include "serversmodel.h"
#include "operationdialog.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

CProfilesDialog::CProfilesDialog(QWidget *parent):QDialog(parent), m_currentProfileIndex(-1)
{
	setupUi(this);

	connect(addButton, SIGNAL(clicked()), SLOT(onAddProfile()));
	connect(deleteButton, SIGNAL(clicked()), SLOT(onDeleteProfile()));
	connect(profilesListView, SIGNAL(clicked(QModelIndex)), SLOT(onProfileClicked(QModelIndex)));
	connect(executableDefaultButton, SIGNAL(clicked()), SLOT(onExecutableDefaultClicked()));
	connect(executableBrowseButton, SIGNAL(clicked()), SLOT(onExecutableBrowseClicked()));
	connect(directoryButton, SIGNAL(clicked()), SLOT(onProfileDirectoryClicked()));

	m_model = new CProfilesModel(this);
	m_serversModel = new CServersModel(this);

	profilesListView->setModel(m_model);
	serverComboBox->setModel(m_serversModel);

	int index = CConfigFile::getInstance()->getDefaultProfileIndex();

	profilesListView->setCurrentIndex(m_model->index(index, 0));
	displayProfile(index);
}

CProfilesDialog::~CProfilesDialog()
{
}

void CProfilesDialog::accept()
{
	saveProfile(m_currentProfileIndex);

	m_model->save();

	QDialog::accept();
}

void CProfilesDialog::onAddProfile()
{
	addProfile();
}

void CProfilesDialog::onDeleteProfile()
{
	QMessageBox::StandardButton res = QMessageBox::question(this, tr("Confirmation"), tr("You're going to delete a profile, files won't be deleted and you'll have to do that manually.\nAre you sure to delete this profile?"));

	if (res != QMessageBox::Yes) return;

	QModelIndex index = profilesListView->currentIndex();

	deleteProfile(index.row());
}

void CProfilesDialog::onProfileClicked(const QModelIndex &index)
{
	qDebug() << "clicked on" << index;

	displayProfile(index.row());
}

void CProfilesDialog::displayProfile(int index)
{
	bool enabled = index > -1;

	profileIdLabel->setEnabled(enabled);
	nameEdit->setEnabled(enabled);
	serverComboBox->setEnabled(enabled);
	argumentsEdit->setEnabled(enabled);
	commentsEdit->setEnabled(enabled);

	if (index < 0) return;

	saveProfile(m_currentProfileIndex);

	const CProfile &profile = m_model->getProfiles()[index];

	QString executable = profile.executable;

	if (executable.isEmpty())
	{
		executable = CConfigFile::getInstance()->getServerClientFullPath(profile.server);
	}

	QString profileDirectory = profile.getDirectory();

	// update all widgets with content of profile
	profileIdLabel->setText(profile.id);
	nameEdit->setText(profile.name);
	serverComboBox->setCurrentIndex(m_serversModel->getIndexFromServerID(profile.server));
	executablePathLabel->setText(QFileInfo(executable).fileName());
	argumentsEdit->setText(profile.arguments);
	commentsEdit->setPlainText(profile.comments);
	directoryPathLabel->setText(profileDirectory);
	desktopShortcutCheckBox->setChecked(profile.desktopShortcut);
	menuShortcutCheckBox->setChecked(profile.menuShortcut);

	// disable click on button if directory doesn't exist
	directoryButton->setEnabled(QFile::exists(profileDirectory));

	updateExecutableVersion(index);

	m_currentProfileIndex = index;
}

void CProfilesDialog::saveProfile(int index)
{
	if (index < 0 || index >= m_model->rowCount()) return;

	CProfile &profile = m_model->getProfiles()[index];
	
	profile.name = nameEdit->text();
	profile.server = m_serversModel->getServerIDFromIndex(serverComboBox->currentIndex());
	profile.arguments = argumentsEdit->text();
	profile.comments = commentsEdit->toPlainText();
	profile.desktopShortcut = desktopShortcutCheckBox->isChecked();
	profile.menuShortcut = menuShortcutCheckBox->isChecked();
}

void CProfilesDialog::deleteProfile(int index)
{
	if (index < 0) return;

	m_model->removeRow(index);

	// decrement profile index
	--index;

	// select row and update content
	profilesListView->setCurrentIndex(m_model->index(index, 0));
	displayProfile(index);

	// TODO: delete files for delete profile
}

void CProfilesDialog::addProfile()
{
	int index = m_model->rowCount();

	// append the new profile
	m_model->insertRow(index);

	CConfigFile *config = CConfigFile::getInstance();

	CProfile &profile = m_model->getProfiles()[index];
	const CServer &server = config->getServer(config->getDefaultServerIndex());

	int nextId = 0;

	// search an ID that doesn't correspond to an existing profile directory
	while (QFile::exists(config->getProfileDirectory() + "/" + QString::number(nextId))) ++nextId;

	// increment this ID until not used in profiles
	while(nextId < 100)
	{
		bool found = false;

		// search if this ID is already used in existing profiles
		foreach(const CProfile &p, m_model->getProfiles())
		{
			if (p.id == QString::number(nextId))
			{
				found = true;
				break;
			}
		}

		if (!found) break;

		// increment ID
		++nextId;
	}

	// set default parameters
	profile.id = QString::number(nextId);
	profile.server = server.id;

	profilesListView->setCurrentIndex(m_model->index(index, 0));
	displayProfile(index);

	// TODO: copy files to new server if files don't exist
}

void CProfilesDialog::updateExecutableVersion(int index)
{
	if (index < 0) return;

	const CProfile &profile = m_model->getProfiles()[index];

	QString executable = profile.executable;

	// file empty, use default one
	if (executable.isEmpty())
	{
		executable += CConfigFile::getInstance()->getServerClientFullPath(profile.server);
	}

	// file doesn't exist
	if (executable.isEmpty() || !QFile::exists(executable)) return;

	// launch executable with --version argument
	QProcess process;
	process.setProcessChannelMode(QProcess::MergedChannels);
	process.start(executable, QStringList() << "--version", QIODevice::ReadWrite);

	if (!process.waitForStarted()) return;

	QByteArray data;

	// read all output
	while (process.waitForReadyRead()) data.append(process.readAll());

	// convert output to string
	QString versionString = QString::fromUtf8(data);

	// parse version from output
	QRegExp reg("([A-Za-z0-1_.]+) ((DEV|FV) ([0-9.]+))");

	if (reg.indexIn(versionString) > -1)
	{
		executablePathLabel->setText(QString("%1 (%2)").arg(QFileInfo(executable).fileName()).arg(reg.cap(2)));
	}
}

void CProfilesDialog::onExecutableDefaultClicked()
{
	if (m_currentProfileIndex < 0) return;

	CProfile &profile = m_model->getProfiles()[m_currentProfileIndex];

	profile.executable.clear();

	updateExecutableVersion(m_currentProfileIndex);
}

void CProfilesDialog::onExecutableBrowseClicked()
{
	if (m_currentProfileIndex < 0) return;

	CProfile &profile = m_model->getProfiles()[m_currentProfileIndex];

	QString executable = profile.executable;

	if (executable.isEmpty())
	{
		executable = CConfigFile::getInstance()->getServerClientFullPath(profile.server);
	}

	executable = QFileDialog::getOpenFileName(this, tr("Please choose Ryzom client executable to launch"), executable, tr("Executables (*.exe)"));

	if (executable.isEmpty()) return;

	// don't need to save the new executable if the same as default one
	if (executable == CConfigFile::getInstance()->getServerClientFullPath(profile.server))
	{
		profile.executable.clear();
	}
	else
	{
		profile.executable = executable;
	}

	executablePathLabel->setText(QFileInfo(executable).fileName());

	updateExecutableVersion(m_currentProfileIndex);
}

void CProfilesDialog::onProfileDirectoryClicked()
{
	if (m_currentProfileIndex < 0) return;

	const CProfile &profile = m_model->getProfiles()[m_currentProfileIndex];

	QString profileDirectory = profile.getDirectory();

	QDesktopServices::openUrl(QUrl::fromLocalFile(profileDirectory));
}
