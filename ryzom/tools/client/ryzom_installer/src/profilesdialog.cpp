// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2019  Winch Gate Property Limited
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
#include "utils.h"

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

	const CProfiles &profiles = m_model->getProfiles();

	// check if profiles are valid
	foreach(const CProfile &profile, profiles)
	{
		QString error;

		if (!profile.isValid(error))
		{
			// display an error message
			QMessageBox::critical(this, tr("Error"), error);
			return;
		}
	}

	m_model->save();

	QDialog::accept();
}

void CProfilesDialog::onAddProfile()
{
	addProfile();
}

void CProfilesDialog::onDeleteProfile()
{
	QMessageBox::StandardButton res = QMessageBox::question(this, tr("Confirmation"), tr("You're going to delete a profile, all files that belong to it (configuration, saves, logs, screenshots, etc...) will be deleted.\nAre you sure to delete this profile?"));

	if (res != QMessageBox::Yes) return;

	QModelIndex index = profilesListView->currentIndex();

	deleteProfile(index.row());
}

void CProfilesDialog::onProfileClicked(const QModelIndex &index)
{
	nlwarning("Clicked on profile %d", index.row());

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
	langComboBox->setEnabled(enabled);

	if (index < 0) return;

	saveProfile(m_currentProfileIndex);

	const CProfile &profile = m_model->getProfiles()[index];

	QString executable = profile.executable;

	if (executable.isEmpty())
	{
		executable = profile.getClientFullPath();
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
	langComboBox->setCurrentIndex(getIndexFromProfileLanguage(profile.language));

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
	profile.language = getProfileLanguageFromIndex(langComboBox->currentIndex());
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

	// delete files for delete profile
	COperationDialog dialog(this);
	dialog.setOperation(OperationUpdateProfiles);
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
	profile.language = config->getLanguage(); // locale

	profilesListView->setCurrentIndex(m_model->index(index, 0));
	displayProfile(index);

	// TODO: copy files to new server if files don't exist
}

void CProfilesDialog::updateExecutableVersion(int index)
{
	if (index < 0) return;

	const CProfile &profile = m_model->getProfiles()[index];
	const CServer &server = CConfigFile::getInstance()->getServer(profile.server);

	QString executable = profile.executable;

	// file empty, use default one
	if (executable.isEmpty())
	{
		executable = server.getClientFullPath();
	}

	// file doesn't exist
	if (executable.isEmpty() || !QFile::exists(executable)) return;

	// convert output to string
	QString versionString = getVersionFromExecutable(executable, server.getDirectory());

	if (!versionString.isEmpty())
	{
		executablePathLabel->setText(QString("%1 (%2)").arg(QFileInfo(executable).fileName()).arg(versionString));
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
	QString defaultExecutable = CConfigFile::getInstance()->getServer(profile.server).getClientFullPath();

	if (executable.isEmpty()) executable = defaultExecutable;

	QString filter;

#ifdef Q_OS_WIN32
	filter = tr("Executables (*.exe)");
#else
	filter = tr("Executables (*)");
#endif

	QFileDialog open;
	open.setFilter(QDir::Executable | QDir::NoDotAndDotDot | QDir::Files);

	executable = open.getOpenFileName(this, tr("Please choose Ryzom client executable to launch"), executable, filter);

	if (executable.isEmpty()) return;

	// don't need to save the new executable if the same as default one
	if (executable == defaultExecutable)
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

int CProfilesDialog::getIndexFromProfileLanguage(const QString &lang) const
{
	if (lang == "en") return 0;
	if (lang == "fr") return 1;
	if (lang == "de") return 2;
	if (lang == "es") return 3;
	if (lang == "ru") return 4;

	return -1;
}

QString CProfilesDialog::getProfileLanguageFromIndex(int index) const
{
	if (index == 0) return "en";
	if (index == 1) return "fr";
	if (index == 2) return "de";
	if (index == 3) return "es";
	if (index == 4) return "ru";
	// locale
	return CConfigFile::getInstance()->getLanguage();
}
