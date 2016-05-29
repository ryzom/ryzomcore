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

	CProfile profile = CConfigFile::getInstance()->getProfile(profileIndex);

	if (profile.executable.isEmpty()) return;

	QStringList arguments;
	arguments << "-p";
	arguments << QString::number(profileIndex);
	arguments << profile.arguments;

	bool started = QProcess::startDetached(profile.executable, arguments);

	CConfigFile::getInstance()->setDefaultProfileIndex(profileIndex);
}

void CMainWindow::onConfigureClicked()
{
	int profileIndex = profilesComboBox->currentIndex();

	if (profileIndex < 0) return;

	CProfile profile = CConfigFile::getInstance()->getProfile(profileIndex);

	if (profile.server.isEmpty()) return;

	CServer server = CConfigFile::getInstance()->getServer(profile.server);

	if (server.configurationFilename.isEmpty()) return;

	QStringList arguments;
	arguments << "-p";
	arguments << QString::number(profileIndex);

	bool started = QProcess::startDetached(server.configurationFilename, arguments);

	CConfigFile::getInstance()->setDefaultProfileIndex(profileIndex);
}

void CMainWindow::onProfiles()
{
	CProfilesDialog dialog;

	if (dialog.exec())
	{
		updateProfiles();
	}
}

void CMainWindow::onUninstall()
{
	CUninstallWizardDialog dialog(this);

	if (dialog.exec())
	{
		COperationDialog dialog(&dialog);

		dialog.setOperation(COperationDialog::OperationUninstall);

		dialog.exec();
	}
}

void CMainWindow::onAbout()
{
	QString br("<br>");

	QMessageBox::about(this,
		tr("About %1").arg("Ryzom Installer"),
		QString("Ryzom Installer %1").arg(QApplication::applicationVersion()) + br +
		tr("Program to install, download and manage Ryzom configurations.") +
		br+br+
		tr("Author: %1").arg("Cedric 'Kervala' OCHS") + br +
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
