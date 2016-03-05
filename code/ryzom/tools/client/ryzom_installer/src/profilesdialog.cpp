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

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

CProfilesDialog::CProfilesDialog():QDialog(), m_currentProfileIndex(-1)
{
	setupUi(this);

	connect(addButton, SIGNAL(clicked()), SLOT(onAddProfile()));
	connect(deleteButton, SIGNAL(clicked()), SLOT(onDeleteProfile()));
	connect(profilesListView, SIGNAL(clicked(QModelIndex)), SLOT(onProfileClicked(QModelIndex)));
	connect(executableBrowseButton, SIGNAL(clicked()), SLOT(onExecutableBrowseClicked()));

	m_model = new CProfilesModel(this);

	profilesListView->setModel(m_model);

	QStringList servers;
	servers << "Atys";
	servers << "Yubo";

	QStringListModel *serversModel = new QStringListModel(servers, this);

	serverComboBox->setModel(serversModel);
}

CProfilesDialog::~CProfilesDialog()
{
}

void CProfilesDialog::accept()
{
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
	if (index < 0) return;

	saveProfile(m_currentProfileIndex);

	const CProfile &profile = m_model->getProfiles()[index];

	profileIdLabel->setText(QString::number(profile.id));
	accountEdit->setText(profile.account);
	nameEdit->setText(profile.name);
	serverComboBox->setCurrentIndex(0);
	executablePathLabel->setText(QFileInfo(profile.executable).fileName());
	argumentsEdit->setText(profile.arguments);
	commentsEdit->setPlainText(profile.comments);
	directoryPathLabel->setText(CConfigFile::getInstance()->getProfileDirectory());

	m_currentProfileIndex = index;
}

void CProfilesDialog::saveProfile(int index)
{
	if (index < 0) return;

	CProfile &profile = m_model->getProfiles()[index];

	profileIdLabel->setText(QString::number(profile.id));
	profile.account = accountEdit->text();
	profile.name = nameEdit->text();
//	serverComboBox->setCurrentIndex(0);
	profile.arguments = argumentsEdit->text();
	profile.comments = commentsEdit->toPlainText();
}

void CProfilesDialog::deleteProfile(int index)
{
	if (index < 0) return;

	m_model->removeRow(index);
}

void CProfilesDialog::addProfile()
{
	// TODO: browse all folders in AppData/Roaming/Ryzom
}

void CProfilesDialog::onExecutableBrowseClicked()
{
	if (m_currentProfileIndex < 0) return;

	CProfile &profile = m_model->getProfiles()[m_currentProfileIndex];

	QString file = QFileDialog::getOpenFileName(this, tr("Please choose Ryzom client executable to launch"), profile.executable, tr("Executables (*.exe)"));

	if (file.isEmpty()) return;

	profile.executable = file;

	executablePathLabel->setText(QFileInfo(profile.executable).fileName());

	QProcess process;
	process.setProcessChannelMode(QProcess::MergedChannels);
	process.start(profile.executable, QStringList() << "--version", QIODevice::ReadWrite);

	if (!process.waitForStarted()) return;

	QByteArray data;

	while (process.waitForReadyRead()) data.append(process.readAll());

	QString versionString = QString::fromUtf8(data);

	QRegExp reg("([A-Za-z0-1_.]+) ((DEV|FV) ([0-9.]+))");

	if (reg.indexIn(versionString) > -1)
	{
		executableVersionLabel->setText(reg.cap(2));
	}

	// ryzom_client_dev_d.exe DEV 0.12.0.7331 (built on 2016-02-25 22:16:50)
	// Copyright (C) 2004-2016 Winchgate and The Ryzom Core Community
}
