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
	connect(executableBrowseButton, SIGNAL(clicked()), SLOT(onExecutableBrowseClicked()));

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
	accountEdit->setEnabled(enabled);
	nameEdit->setEnabled(enabled);
	serverComboBox->setEnabled(enabled);
	argumentsEdit->setEnabled(enabled);
	commentsEdit->setEnabled(enabled);

	if (index < 0) return;

	saveProfile(m_currentProfileIndex);

	const CProfile &profile = m_model->getProfiles()[index];

	// update all widgets with content of profile
	profileIdLabel->setText(profile.id);
	accountEdit->setText(profile.account);
	nameEdit->setText(profile.name);
	serverComboBox->setCurrentIndex(m_serversModel->getIndexFromServerID(profile.server));
	executablePathLabel->setText(QFileInfo(profile.executable).fileName());
	argumentsEdit->setText(profile.arguments);
	commentsEdit->setPlainText(profile.comments);
	directoryPathLabel->setText(CConfigFile::getInstance()->getProfileDirectory());
	desktopShortcutCheckBox->setChecked(profile.desktopShortcut);
	menuShortcutCheckBox->setChecked(profile.menuShortcut);

	updateExecutableVersion(index);

	m_currentProfileIndex = index;
}

void CProfilesDialog::saveProfile(int index)
{
	if (index < 0) return;

	CProfile &profile = m_model->getProfiles()[index];
	
	profile.account = accountEdit->text();
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

	COperationDialog dialog;
}

void CProfilesDialog::addProfile()
{
	// TODO: browse all folders in AppData/Roaming/Ryzom
}

void CProfilesDialog::updateExecutableVersion(int index)
{
	if (index < 0) return;

	const CProfile &profile = m_model->getProfiles()[index];

	QString executable = profile.executable;

	// file empty, use default one
	if (executable.isEmpty())
	{
		executable = CConfigFile::getInstance()->getInstallationDirectory() + "/" + profile.server + "/";

#if defined(Q_OS_WIN32)
		executable += "ryzom_client_r.exe";
#elif defined(Q_OS_APPLE)
		executable += "Ryzom.app/Contents/MacOS/Ryzom";
#else
		executable += "ryzom_client";
#endif
	}

	// file doesn't exist
	if (!QFile::exists(executable)) return;

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
		executableVersionLabel->setText(reg.cap(2));
	}
}

void CProfilesDialog::onExecutableBrowseClicked()
{
	if (m_currentProfileIndex < 0) return;

	CProfile &profile = m_model->getProfiles()[m_currentProfileIndex];

	QString file = QFileDialog::getOpenFileName(this, tr("Please choose Ryzom client executable to launch"), profile.executable, tr("Executables (*.exe)"));

	if (file.isEmpty()) return;

	profile.executable = file;

	executablePathLabel->setText(QFileInfo(profile.executable).fileName());

	updateExecutableVersion(m_currentProfileIndex);
}
