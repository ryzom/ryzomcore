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
#include "mainwindow.h"
#include "downloader.h"
#include "profilesdialog.h"
#include "uninstallwizarddialog.h"
#include "operationdialog.h"
#include "configfile.h"
#include "config.h"
#include "profilesmodel.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

CMainWindow::CMainWindow():QMainWindow()
{
	setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);

	// downloader
	m_downloader = new CDownloader(this);

	connect(m_downloader, SIGNAL(htmlPageContent(QString)), SLOT(onHtmlPageContent(QString)));

	connect(actionProfiles, SIGNAL(triggered()), SLOT(onProfiles()));
	connect(actionUninstall, SIGNAL(triggered()), SLOT(onUninstall()));
	connect(actionQuit, SIGNAL(triggered()), SLOT(onQuit()));

	connect(playButton, SIGNAL(clicked()), SLOT(onPlayClicked()));
	connect(configureButton, SIGNAL(clicked()), SLOT(onConfigureClicked()));

	connect(actionAboutQt, SIGNAL(triggered()), SLOT(onAboutQt()));
	connect(actionAbout, SIGNAL(triggered()), SLOT(onAbout()));

	connect(profilesComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onProfileChanged(int)));

	updateProfiles();
}

CMainWindow::~CMainWindow()
{
}

void CMainWindow::showEvent(QShowEvent *e)
{
	e->accept();
}

void CMainWindow::closeEvent(QCloseEvent *e)
{
	hide();

	e->accept();
}

void CMainWindow::updateProfiles()
{
	profilesComboBox->setModel(new CProfilesModel(this));
}

void CMainWindow::onPlayClicked()
{
	int profileIndex = profilesComboBox->currentIndex();

	if (profileIndex < 0) return;

	CConfigFile *config = CConfigFile::getInstance();

	const CProfile &profile = config->getProfile(profileIndex);

	QString executable = config->getProfileClientFullPath(profileIndex);

	if (executable.isEmpty() || !QFile::exists(executable)) return;

	QStringList arguments;
	arguments << "-p";
	arguments << profile.id;
	arguments << profile.arguments.split(' ');

	bool started = QProcess::startDetached(executable, arguments);

	CConfigFile::getInstance()->setDefaultProfileIndex(profileIndex);
}

void CMainWindow::onConfigureClicked()
{
	int profileIndex = profilesComboBox->currentIndex();

	if (profileIndex < 0) return;

	CConfigFile *config = CConfigFile::getInstance();

	const CProfile &profile = config->getProfile(profileIndex);

	QString executable = config->getServerConfigurationFullPath(profile.server);

	if (executable.isEmpty() || !QFile::exists(executable)) return;

	QStringList arguments;
	arguments << "-p";
	arguments << profile.id;

	bool started = QProcess::startDetached(executable, arguments);

	CConfigFile::getInstance()->setDefaultProfileIndex(profileIndex);
}

void CMainWindow::onProfiles()
{
	bool updated = false;

	{
		CProfilesDialog dialog(this);

		if (dialog.exec())
		{
			updateProfiles();

			updated = true;
		}
	}

	if (updated)
	{
		COperationDialog dialog(this);

		dialog.setOperation(COperationDialog::OperationUpdateProfiles);

		if (!dialog.exec())
		{
			// aborted
		}
	}
}

void CMainWindow::onUninstall()
{
	CConfigFile *config = CConfigFile::getInstance();

	SUninstallComponents components;

	// add all servers by default
	for (int i = 0; i < config->getServersCount(); ++i)
	{
		components.servers << i;
	}

	{
		CUninstallWizardDialog dialog(this);

		dialog.setSelectedComponents(components);

		if (!dialog.exec()) return;

		components = dialog.getSelectedCompenents();
	}

	COperationDialog dialog;

	dialog.setOperation(COperationDialog::OperationUninstall);
	dialog.setUninstallComponents(components);

	if (dialog.exec())
	{
	}
}

void CMainWindow::onQuit()
{
	close();
}

void CMainWindow::onAbout()
{
	QString br("<br>");

	QMessageBox::about(this,
		tr("About %1").arg("Ryzom Installer"),
		QString("Ryzom Installer %1").arg(QApplication::applicationVersion()) + br +
		tr("Program to install, download and manage Ryzom profiles.") +
		br+br+
		tr("Author: %1").arg("C&eacute;dric 'Kervala' OCHS") + br +
		tr("Copyright: %1").arg(COPYRIGHT) + br +
		tr("Support: %1").arg("<a href=\"https://bitbucket.org/ryzom/ryzomcore/issues?status=new&status=open\">Ryzom Core on Bitbucket</a>"));
}

void CMainWindow::onAboutQt()
{
	QMessageBox::aboutQt(this);
}

void CMainWindow::onHtmlPageContent(const QString &html)
{
	htmlTextEdit->setHtml(html);
}

void CMainWindow::onProfileChanged(int profileIndex)
{
	if (profileIndex < 0) return;

	CConfigFile *config = CConfigFile::getInstance();

	CProfile profile = config->getProfile(profileIndex);
	CServer server = config->getServer(profile.server);

	// load changelog
	m_downloader->getHtmlPageContent(config->expandVariables(server.displayUrl));
}
